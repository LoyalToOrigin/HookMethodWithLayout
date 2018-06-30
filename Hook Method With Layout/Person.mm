//
//  Person.m
//  Hook Method With Layout
//
//  Created by 谢晨 on 2018/6/26.
//  Copyright © 2018 ricky. All rights reserved.
//

#import "Person.h"
#import <objc/runtime.h>
#import "ClassInfo.h"

@implementation Person

//struct objc_method { // old, OBJC2_UNAVAILABLE
//    SEL method_name;        // 方法名称
//    charchar *method_typesE;    // 参数和返回类型的描述字串
//    IMP method_imp;         // 方法的具体的实现的指针
//}


//struct method_t { // new
//    SEL name;
//    const char *types;
//    IMP imp;
//    struct SortBySELAddress :
//    public std::binary_function<const method_t&,
//    const method_t&, bool>
//    {
//        bool operator() (const method_t& lhs,
//                         const method_t& rhs)
//        { return lhs.name < rhs.name; }
//    };
//};

//struct method_t {
//    SEL name;
//    const char *types;
//    IMP imp;
//};





//+ (void)initialize { // method->imp = (IMP)hookedSayHello; 直接修改 imp 没问题, load直接修改就有问题
+ (void)load {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        Class aClass = [self class];
//        Class aClass = self; // 不给self发消息, cache不会生成, 结果就和我们的预想一样
        
        SEL originalSelector = @selector(sayHello);
        
        Method originalMethod = class_getInstanceMethod(aClass, originalSelector);
        
//        method_setImplementation(originalMethod, (IMP)hookedSayHello); //Runtime API, 可以发现cache被清除了, 可以打开注释, 验证结果
        
        struct method_t *method = (struct method_t *)originalMethod;
        method->imp = (IMP)hookedSayHello;
        
        // cache问题, 因为 已经和 imp缓存了, 直接会调用原来方法
        // method_setImplementation 中有个函数 flushCache -> cache_erase_nolock, 会重新设置 cache
        
        // 修改cache
        struct my_objc_class *clz = (__bridge struct my_objc_class *)aClass;
        uint32_t cacheCount = clz->cache.capacity();
        NSLog(@"cacheCount : %d", cacheCount);
        
        
        for (NSInteger i = 0; i < cacheCount; i++) {
            char *key = (char *)((clz->cache._buckets + i)->_key);
            // 这里设置一下
            printf("%ld - %s\n", i, key); // 测试
            
            if (key) {
                NSString *selectorName = [NSString stringWithUTF8String:key];

                if ([selectorName isEqualToString:@"sayHello"]) {
                    (clz->cache._buckets + i)->_imp = (IMP)hookedSayHello;
                }
            }
        }
    });
}





- (void)sayHello {
    NSLog(@"Hello, everybody!");
}


void hookedSayHello (id self, SEL _cmd, ...) {
    NSLog(@"This is hooked sayHello");
}


























//- (void)hookedSayHello {
//    NSLog(@"This is hooked sayHello");
//}


@end
