#ifndef _MOPSA_AUX_H
#define _MOPSA_AUX_H


extern signed char _mopsa_rand_s8();
extern unsigned char _mopsa_rand_u8();

extern signed short _mopsa_rand_s16();
extern unsigned short _mopsa_rand_u16();

extern signed int _mopsa_rand_s32();
extern unsigned int _mopsa_rand_u32();

extern signed long _mopsa_rand_s64();
extern unsigned long _mopsa_rand_u64();

extern float _mopsa_rand_float();
extern double _mopsa_rand_double();

extern void *_mopsa_rand_void_pointer();


// builtin type
typedef uintnat uintptr_t;

#endif // _MOPSA_AUX_H
