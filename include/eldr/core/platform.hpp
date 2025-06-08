#pragma once

// Reduce namespace pollution from windows.h
#if defined(_WIN32)
#  if !defined(WIN32_LEAN_AND_MEAN)
#    define WIN32_LEAN_AND_MEAN
#  endif
#  if !defined(_USE_MATH_DEFINES)
#    define _USE_MATH_DEFINES
#  endif
#endif

// Likely/unlikely macros (only on GCC/Clang)
#if defined(__GNUG__) || defined(__clang__)
#  define likely(x) __builtin_expect(!!(x), 1)
#  define unlikely(x) __builtin_expect(!!(x), 0)
#else
#  define likely(x) (x)
#  define unlikely(x) (x)
#endif

// Processor architecture
#if defined(_MSC_VER) && defined(_M_X86)
#  error 32-bit builds are not supported.
#endif

#if defined(_MSC_VER)
#  define MI_NOINLINE __declspec(noinline)
#  define MI_INLINE __forceinline
#else
#  define EL_NOINLINE __attribute__((noinline))
#  define EL_INLINE __attribute__((always_inline)) inline
#endif

// clang-format off
#define EL_EXPAND(x) x

#define EL_VA_SIZE_3(_, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, \
        _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, \
        _27, _28, _29, _30, _31, N, ...) N
#define EL_VA_SIZE_2(...) EL_EXPAND(EL_VA_SIZE_3(__VA_ARGS__))

#if defined(__GNUC__) && !defined(__clang__)
#  define EL_VA_SIZE_1(...) _ __VA_OPT__(,) __VA_ARGS__ , \
    31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, \
    16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
#else
#  define EL_VA_SIZE_1(...) _, ##__VA_ARGS__ , \
    31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, \
    16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
#endif
#define EL_VA_SIZE(...) EL_VA_SIZE_2(EL_VA_SIZE_1(__VA_ARGS__))

#define EL_MAPN_0(...)
#define EL_MAPC_0(...)
#define EL_MAPN_1(Z,a) Z(a)
#define EL_MAPC_1(Z,a) Z(a)
#define EL_MAPN_2(Z,a,b) Z(a)Z(b)
#define EL_MAPC_2(Z,a,b) Z(a),Z(b)
#define EL_MAPN_3(Z,a,b,c) Z(a)Z(b)Z(c)
#define EL_MAPC_3(Z,a,b,c) Z(a),Z(b),Z(c)
#define EL_MAPN_4(Z,a,b,c,d) Z(a)Z(b)Z(c)Z(d)
#define EL_MAPC_4(Z,a,b,c,d) Z(a),Z(b),Z(c),Z(d)
#define EL_MAPN_5(Z,a,b,c,d,e) Z(a)Z(b)Z(c)Z(d)Z(e)
#define EL_MAPC_5(Z,a,b,c,d,e) Z(a),Z(b),Z(c),Z(d),Z(e)
#define EL_MAPN_6(Z,a,b,c,d,e,f) Z(a)Z(b)Z(c)Z(d)Z(e)Z(f)
#define EL_MAPC_6(Z,a,b,c,d,e,f) Z(a),Z(b),Z(c),Z(d),Z(e),Z(f)
#define EL_MAPN_7(Z,a,b,c,d,e,f,g) Z(a)Z(b)Z(c)Z(d)Z(e)Z(f)Z(g)
#define EL_MAPC_7(Z,a,b,c,d,e,f,g) Z(a),Z(b),Z(c),Z(d),Z(e),Z(f),Z(g)
#define EL_MAPN_8(Z,a,b,c,d,e,f,g,h) Z(a)Z(b)Z(c)Z(d)Z(e)Z(f)Z(g)Z(h)
#define EL_MAPC_8(Z,a,b,c,d,e,f,g,h) Z(a),Z(b),Z(c),Z(d),Z(e),Z(f),Z(g),Z(h)
#define EL_MAPN_9(Z,a,b,c,d,e,f,g,h,i) Z(a)Z(b)Z(c)Z(d)Z(e)Z(f)Z(g)Z(h)Z(i)
#define EL_MAPC_9(Z,a,b,c,d,e,f,g,h,i) Z(a),Z(b),Z(c),Z(d),Z(e),Z(f),Z(g),Z(h),Z(i)
#define EL_MAPN_10(Z,a,b,c,d,e,f,g,h,i,j) Z(a)Z(b)Z(c)Z(d)Z(e)Z(f)Z(g)Z(h)Z(i)Z(j)
#define EL_MAPC_10(Z,a,b,c,d,e,f,g,h,i,j) Z(a),Z(b),Z(c),Z(d),Z(e),Z(f),Z(g),Z(h),Z(i),Z(j)
#define EL_MAPN_11(Z,a,b,c,d,e,f,g,h,i,j,k) Z(a)Z(b)Z(c)Z(d)Z(e)Z(f)Z(g)Z(h)Z(i)Z(j)Z(k)
#define EL_MAPC_11(Z,a,b,c,d,e,f,g,h,i,j,k) Z(a),Z(b),Z(c),Z(d),Z(e),Z(f),Z(g),Z(h),Z(i),Z(j),Z(k)
#define EL_MAPN_12(Z,a,b,c,d,e,f,g,h,i,j,k,l) Z(a)Z(b)Z(c)Z(d)Z(e)Z(f)Z(g)Z(h)Z(i)Z(j)Z(k)Z(l)
#define EL_MAPC_12(Z,a,b,c,d,e,f,g,h,i,j,k,l) Z(a),Z(b),Z(c),Z(d),Z(e),Z(f),Z(g),Z(h),Z(i),Z(j),Z(k),Z(l)
#define EL_MAPN_13(Z,a,b,c,d,e,f,g,h,i,j,k,l,m) Z(a)Z(b)Z(c)Z(d)Z(e)Z(f)Z(g)Z(h)Z(i)Z(j)Z(k)Z(l)Z(m)
#define EL_MAPC_13(Z,a,b,c,d,e,f,g,h,i,j,k,l,m) Z(a),Z(b),Z(c),Z(d),Z(e),Z(f),Z(g),Z(h),Z(i),Z(j),Z(k),Z(l),Z(m)
#define EL_MAPN_14(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n) Z(a)Z(b)Z(c)Z(d)Z(e)Z(f)Z(g)Z(h)Z(i)Z(j)Z(k)Z(l)Z(m)Z(n)
#define EL_MAPC_14(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n) Z(a),Z(b),Z(c),Z(d),Z(e),Z(f),Z(g),Z(h),Z(i),Z(j),Z(k),Z(l),Z(m),Z(n)
#define EL_MAPN_15(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o) Z(a)Z(b)Z(c)Z(d)Z(e)Z(f)Z(g)Z(h)Z(i)Z(j)Z(k)Z(l)Z(m)Z(n)Z(o)
#define EL_MAPC_15(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o) Z(a),Z(b),Z(c),Z(d),Z(e),Z(f),Z(g),Z(h),Z(i),Z(j),Z(k),Z(l),Z(m),Z(n),Z(o)
#define EL_MAPN_16(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) Z(a)Z(b)Z(c)Z(d)Z(e)Z(f)Z(g)Z(h)Z(i)Z(j)Z(k)Z(l)Z(m)Z(n)Z(o)Z(p)
#define EL_MAPC_16(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) Z(a),Z(b),Z(c),Z(d),Z(e),Z(f),Z(g),Z(h),Z(i),Z(j),Z(k),Z(l),Z(m),Z(n),Z(o),Z(p)
#define EL_MAPN_17(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q) Z(a)Z(b)Z(c)Z(d)Z(e)Z(f)Z(g)Z(h)Z(i)Z(j)Z(k)Z(l)Z(m)Z(n)Z(o)Z(p)Z(q)
#define EL_MAPC_17(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q) Z(a),Z(b),Z(c),Z(d),Z(e),Z(f),Z(g),Z(h),Z(i),Z(j),Z(k),Z(l),Z(m),Z(n),Z(o),Z(p),Z(q)
#define EL_MAPN_18(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r) Z(a)Z(b)Z(c)Z(d)Z(e)Z(f)Z(g)Z(h)Z(i)Z(j)Z(k)Z(l)Z(m)Z(n)Z(o)Z(p)Z(q)Z(r)
#define EL_MAPC_18(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r) Z(a),Z(b),Z(c),Z(d),Z(e),Z(f),Z(g),Z(h),Z(i),Z(j),Z(k),Z(l),Z(m),Z(n),Z(o),Z(p),Z(q),Z(r)
#define EL_MAPN_19(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s) Z(a)Z(b)Z(c)Z(d)Z(e)Z(f)Z(g)Z(h)Z(i)Z(j)Z(k)Z(l)Z(m)Z(n)Z(o)Z(p)Z(q)Z(r)Z(s)
#define EL_MAPC_19(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s) Z(a),Z(b),Z(c),Z(d),Z(e),Z(f),Z(g),Z(h),Z(i),Z(j),Z(k),Z(l),Z(m),Z(n),Z(o),Z(p),Z(q),Z(r),Z(s)
#define EL_MAPN_20(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t) Z(a)Z(b)Z(c)Z(d)Z(e)Z(f)Z(g)Z(h)Z(i)Z(j)Z(k)Z(l)Z(m)Z(n)Z(o)Z(p)Z(q)Z(r)Z(s)Z(t)
#define EL_MAPC_20(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t) Z(a),Z(b),Z(c),Z(d),Z(e),Z(f),Z(g),Z(h),Z(i),Z(j),Z(k),Z(l),Z(m),Z(n),Z(o),Z(p),Z(q),Z(r),Z(s),Z(t)
#define EL_MAPN_21(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u) Z(a)Z(b)Z(c)Z(d)Z(e)Z(f)Z(g)Z(h)Z(i)Z(j)Z(k)Z(l)Z(m)Z(n)Z(o)Z(p)Z(q)Z(r)Z(s)Z(t)Z(u)
#define EL_MAPC_21(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u) Z(a),Z(b),Z(c),Z(d),Z(e),Z(f),Z(g),Z(h),Z(i),Z(j),Z(k),Z(l),Z(m),Z(n),Z(o),Z(p),Z(q),Z(r),Z(s),Z(t),Z(u)
#define EL_MAPN_22(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v) Z(a)Z(b)Z(c)Z(d)Z(e)Z(f)Z(g)Z(h)Z(i)Z(j)Z(k)Z(l)Z(m)Z(n)Z(o)Z(p)Z(q)Z(r)Z(s)Z(t)Z(u)Z(v)
#define EL_MAPC_22(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v) Z(a),Z(b),Z(c),Z(d),Z(e),Z(f),Z(g),Z(h),Z(i),Z(j),Z(k),Z(l),Z(m),Z(n),Z(o),Z(p),Z(q),Z(r),Z(s),Z(t),Z(u),Z(v)
#define EL_MAPN_23(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w) Z(a)Z(b)Z(c)Z(d)Z(e)Z(f)Z(g)Z(h)Z(i)Z(j)Z(k)Z(l)Z(m)Z(n)Z(o)Z(p)Z(q)Z(r)Z(s)Z(t)Z(u)Z(v)Z(w)
#define EL_MAPC_23(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w) Z(a),Z(b),Z(c),Z(d),Z(e),Z(f),Z(g),Z(h),Z(i),Z(j),Z(k),Z(l),Z(m),Z(n),Z(o),Z(p),Z(q),Z(r),Z(s),Z(t),Z(u),Z(v),Z(w)
#define EL_MAPN_24(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x) Z(a)Z(b)Z(c)Z(d)Z(e)Z(f)Z(g)Z(h)Z(i)Z(j)Z(k)Z(l)Z(m)Z(n)Z(o)Z(p)Z(q)Z(r)Z(s)Z(t)Z(u)Z(v)Z(w)Z(x)
#define EL_MAPC_24(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x) Z(a),Z(b),Z(c),Z(d),Z(e),Z(f),Z(g),Z(h),Z(i),Z(j),Z(k),Z(l),Z(m),Z(n),Z(o),Z(p),Z(q),Z(r),Z(s),Z(t),Z(u),Z(v),Z(w),Z(x)
#define EL_MAPN_25(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y) Z(a)Z(b)Z(c)Z(d)Z(e)Z(f)Z(g)Z(h)Z(i)Z(j)Z(k)Z(l)Z(m)Z(n)Z(o)Z(p)Z(q)Z(r)Z(s)Z(t)Z(u)Z(v)Z(w)Z(x)Z(y)
#define EL_MAPC_25(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y) Z(a),Z(b),Z(c),Z(d),Z(e),Z(f),Z(g),Z(h),Z(i),Z(j),Z(k),Z(l),Z(m),Z(n),Z(o),Z(p),Z(q),Z(r),Z(s),Z(t),Z(u),Z(v),Z(w),Z(x),Z(y)
#define EL_MAPN_26(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z) Z(a)Z(b)Z(c)Z(d)Z(e)Z(f)Z(g)Z(h)Z(i)Z(j)Z(k)Z(l)Z(m)Z(n)Z(o)Z(p)Z(q)Z(r)Z(s)Z(t)Z(u)Z(v)Z(w)Z(x)Z(y)Z(z)
#define EL_MAPC_26(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z) Z(a),Z(b),Z(c),Z(d),Z(e),Z(f),Z(g),Z(h),Z(i),Z(j),Z(k),Z(l),Z(m),Z(n),Z(o),Z(p),Z(q),Z(r),Z(s),Z(t),Z(u),Z(v),Z(w),Z(x),Z(y),Z(z)
#define EL_MAPN_27(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A) Z(a)Z(b)Z(c)Z(d)Z(e)Z(f)Z(g)Z(h)Z(i)Z(j)Z(k)Z(l)Z(m)Z(n)Z(o)Z(p)Z(q)Z(r)Z(s)Z(t)Z(u)Z(v)Z(w)Z(x)Z(y)Z(z)Z(A)
#define EL_MAPC_27(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A) Z(a),Z(b),Z(c),Z(d),Z(e),Z(f),Z(g),Z(h),Z(i),Z(j),Z(k),Z(l),Z(m),Z(n),Z(o),Z(p),Z(q),Z(r),Z(s),Z(t),Z(u),Z(v),Z(w),Z(x),Z(y),Z(z),Z(A)
#define EL_MAPN_28(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B) Z(a)Z(b)Z(c)Z(d)Z(e)Z(f)Z(g)Z(h)Z(i)Z(j)Z(k)Z(l)Z(m)Z(n)Z(o)Z(p)Z(q)Z(r)Z(s)Z(t)Z(u)Z(v)Z(w)Z(x)Z(y)Z(z)Z(A)Z(B)
#define EL_MAPC_28(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B) Z(a),Z(b),Z(c),Z(d),Z(e),Z(f),Z(g),Z(h),Z(i),Z(j),Z(k),Z(l),Z(m),Z(n),Z(o),Z(p),Z(q),Z(r),Z(s),Z(t),Z(u),Z(v),Z(w),Z(x),Z(y),Z(z),Z(A),Z(B)
#define EL_MAPN_29(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C) Z(a)Z(b)Z(c)Z(d)Z(e)Z(f)Z(g)Z(h)Z(i)Z(j)Z(k)Z(l)Z(m)Z(n)Z(o)Z(p)Z(q)Z(r)Z(s)Z(t)Z(u)Z(v)Z(w)Z(x)Z(y)Z(z)Z(A)Z(B)Z(C)
#define EL_MAPC_29(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C) Z(a),Z(b),Z(c),Z(d),Z(e),Z(f),Z(g),Z(h),Z(i),Z(j),Z(k),Z(l),Z(m),Z(n),Z(o),Z(p),Z(q),Z(r),Z(s),Z(t),Z(u),Z(v),Z(w),Z(x),Z(y),Z(z),Z(A),Z(B),Z(C)
#define EL_MAPN_30(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D) Z(a)Z(b)Z(c)Z(d)Z(e)Z(f)Z(g)Z(h)Z(i)Z(j)Z(k)Z(l)Z(m)Z(n)Z(o)Z(p)Z(q)Z(r)Z(s)Z(t)Z(u)Z(v)Z(w)Z(x)Z(y)Z(z)Z(A)Z(B)Z(C)Z(D)
#define EL_MAPC_30(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D) Z(a),Z(b),Z(c),Z(d),Z(e),Z(f),Z(g),Z(h),Z(i),Z(j),Z(k),Z(l),Z(m),Z(n),Z(o),Z(p),Z(q),Z(r),Z(s),Z(t),Z(u),Z(v),Z(w),Z(x),Z(y),Z(z),Z(A),Z(B),Z(C),Z(D)
#define EL_MAPN_31(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E) Z(a)Z(b)Z(c)Z(d)Z(e)Z(f)Z(g)Z(h)Z(i)Z(j)Z(k)Z(l)Z(m)Z(n)Z(o)Z(p)Z(q)Z(r)Z(s)Z(t)Z(u)Z(v)Z(w)Z(x)Z(y)Z(z)Z(A)Z(B)Z(C)Z(D)Z(E)
#define EL_MAPC_31(Z,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E) Z(a),Z(b),Z(c),Z(d),Z(e),Z(f),Z(g),Z(h),Z(i),Z(j),Z(k),Z(l),Z(m),Z(n),Z(o),Z(p),Z(q),Z(r),Z(s),Z(t),Z(u),Z(v),Z(w),Z(x),Z(y),Z(z),Z(A),Z(B),Z(C),Z(D),Z(E)

#define EL_CONCAT_(a,b) a ## b
#define EL_CONCAT(a,b) EL_CONCAT_(a,b)
#define EL_MAP_(M, Z, ...) EL_EXPAND(M(Z, __VA_ARGS__))
#define EL_MAP(Z, ...)  EL_MAP_(EL_CONCAT(EL_MAPN_, EL_VA_SIZE(__VA_ARGS__)), Z, __VA_ARGS__)
#define EL_MAPC(Z, ...) EL_MAP_(EL_CONCAT(EL_MAPC_, EL_VA_SIZE(__VA_ARGS__)), Z, __VA_ARGS__)
