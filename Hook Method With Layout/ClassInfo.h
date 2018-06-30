//
//  ClassInfo.h
//  Hook Method With Layout
//
//  Created by 谢晨 on 2018/6/26.
//  Copyright © 2018 ricky. All rights reserved.
//

#import <Foundation/Foundation.h>

#ifndef ClassInfo_h
#define ClassInfo_h

# if __arm64__
#   define ISA_MASK        0x0000000ffffffff8ULL
# elif __x86_64__
#   define ISA_MASK        0x00007ffffffffff8ULL
# endif

#if __LP64__
typedef uint32_t mask_t;
#else
typedef uint16_t mask_t;
#endif
typedef uintptr_t cache_key_t;

struct bucket_t {
    cache_key_t _key;
    IMP _imp;
};

struct cache_t {
    bucket_t *_buckets;
    mask_t _mask;
    mask_t _occupied;
public:
    struct bucket_t *buckets();
    mask_t mask();
    mask_t occupied();
    void incrementOccupied();
    void setBucketsAndMask(struct bucket_t *newBuckets, mask_t newMask);
    void initializeToEmpty();
    
    mask_t capacity();
    bool isConstantEmptyCache();
    bool canBeFreed();
    
    static size_t bytesForCapacity(uint32_t cap);
    static struct bucket_t * endMarker(struct bucket_t *b, uint32_t cap);
    
    void expand();
    void reallocate(mask_t oldCapacity, mask_t newCapacity);
    struct bucket_t * find(cache_key_t key, id receiver);
    
    static void bad_cache(id receiver, SEL sel, Class isa) __attribute__((noreturn));
};

mask_t cache_t::mask()
{
    return _mask;
}

mask_t cache_t::capacity()
{
    return mask() ? mask()+1 : 0;
}

struct entsize_list_tt {
    uint32_t entsizeAndFlags;
    uint32_t count;
};

struct method_t {
    SEL name;
    const char *types;
    IMP imp;
};

struct method_list_t : entsize_list_tt {
    method_t first;
};

struct ivar_t {
    int32_t *offset;
    const char *name;
    const char *type;
    uint32_t alignment_raw;
    uint32_t size;
};

struct ivar_list_t : entsize_list_tt {
    ivar_t first;
};

struct property_t {
    const char *name;
    const char *attributes;
};

struct property_list_t : entsize_list_tt {
    property_t first;
};

struct chained_property_list {
    chained_property_list *next;
    uint32_t count;
    property_t list[0];
};

typedef uintptr_t protocol_ref_t;
struct protocol_list_t {
    uintptr_t count;
    protocol_ref_t list[0];
};

struct class_ro_t {
    uint32_t flags;
    uint32_t instanceStart;
    uint32_t instanceSize;
#ifdef __LP64__
    uint32_t reserved;
#endif
    
    const uint8_t * ivarLayout;
    
    const char * name;
    method_list_t * baseMethodList;
    protocol_list_t * baseProtocols;
    const ivar_list_t * ivars;
    
    const uint8_t * weakIvarLayout;
    property_list_t *baseProperties;
    
    method_list_t *baseMethods() const {
        return baseMethodList;
    }
};

struct class_rw_t {
    uint32_t flags;
    uint32_t version;
    const class_ro_t *ro;
    method_list_t * methods;
    property_list_t *properties;
    const protocol_list_t * protocols;
    Class firstSubclass;
    Class nextSiblingClass;
    char *demangledName;
};


#define FAST_DATA_MASK          0x00007ffffffffff8UL
struct class_data_bits_t {
    uintptr_t bits;
public:
    class_rw_t* data() {
        return (class_rw_t *)(bits & FAST_DATA_MASK);
    }
};


struct my_objc_object {
    void *isa;
};


struct my_objc_class : my_objc_object {
    Class superclass;
    cache_t cache;
    class_data_bits_t bits;
public:
    class_rw_t* data() {
        return bits.data();
    }
    
    my_objc_class* metaClass() {
        return (my_objc_class *)((long long)isa & ISA_MASK);
    }
};
#endif
