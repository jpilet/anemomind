// (c) 2014 Julien Pilet <julien.pilet@gmail.com>

#ifndef DEVICE_FIXED_POINT_H
#define DEVICE_FIXED_POINT_H

#include <stdint.h>
#include <cmath>

// workaround some arduino #define..
#ifdef round
#undef round
#endif

template <typename StoreType, typename LongType, int Shift>
class FixedPoint {
 public:
   // typedefs
   typedef FixedPoint<StoreType, LongType, Shift> ThisType;

   // Constructors
   FixedPoint() { }
   FixedPoint(short x) { _value = construct(x); }
   FixedPoint(int x) { _value = construct(x); }
   FixedPoint(long x) { _value = construct(x); }
   FixedPoint(float x);
   FixedPoint(double x);
   FixedPoint(const ThisType &a) : _value(a._value) { }

   template <typename OtherStoreType, typename OtherLongType, int OtherShift>
   FixedPoint(const FixedPoint<OtherStoreType, OtherLongType, OtherShift> &a)
     : _value(ThisType::convert(a)._value) { }
       

   // Named constructors
   static ThisType rightShiftAndConstruct(LongType x, int shift) {
     LongType val = x;
     if (shift > Shift) {
       val >>= (shift - Shift);
     }
     if (Shift > shift) {
       val <<= (Shift - shift);
     }
     return make(val);
   }

   // Convert any FixedPoint to our type.
   template <typename FromStoreType, typename FromLongType, int FromShift>
   static ThisType convert(FixedPoint<FromStoreType, FromLongType, FromShift> a) {
     return rightShiftAndConstruct(a.rawFixedPoint(), FromShift);
   }

   StoreType round() const { return (_value + (1 << (Shift - 1))) >> Shift; }

   // Casts
   operator short() const { return short(round()); }
   operator int() const { return int(round()); }
   operator float() const { return float(_value) / float(StoreType(1) << Shift); }
   operator double() const { return double(_value) / double(StoreType(1) << Shift); }

   template <typename OtherStoreType, typename OtherLongType, int OtherShift>
   operator FixedPoint<OtherStoreType, OtherLongType, OtherShift>() const {
     return FixedPoint<OtherStoreType, OtherLongType, OtherShift>::convert(*this);
   }

   // const operators
   ThisType operator + (ThisType other) const {
     return make(_value + other._value);
   }
   ThisType operator - (ThisType other) const {
     return make(_value - other._value);
   }
   ThisType operator * (ThisType other) const {
     return make(StoreType((LongType(_value) * LongType(other._value)) >> Shift));
   }
   ThisType operator / (ThisType other) const {
     return make(StoreType((LongType(_value) << Shift) / LongType(other._value)));
   }
   bool operator < (ThisType other) const { return _value < other._value; }
   bool operator <= (ThisType other) const { return _value <= other._value; }
   bool operator == (ThisType other) const { return _value == other._value; }
   bool operator > (ThisType other) const { return _value > other._value; }
   bool operator >= (ThisType other) const { return _value >= other._value; }

   // in-place operators
   ThisType &operator += (ThisType other) { _value += other._value; return *this; }
   ThisType &operator -= (ThisType other) { _value -= other._value; return *this; }
   ThisType &operator *= (ThisType other) {
     _value = StoreType((LongType(_value) * LongType(other._value)) >> Shift);
     return *this; 
   }
   ThisType &operator /= (ThisType other) {
     _value = StoreType((LongType(_value) << Shift) / LongType(other._value));
     return *this; 
   }

   // Useful for serialization, when we need to access its internal representation.
   // Otherwise, these should be avoided.
   StoreType rawFixedPoint() const { return _value; }
   static ThisType make(StoreType value) {
     ThisType result;
     result._value = value;
     return result;
   }
 private:
   static StoreType construct(StoreType x) { return x << Shift; }
   StoreType _value;
};

typedef FixedPoint<int16_t, int32_t, 8> FP8_8;
typedef FixedPoint<int32_t, int64_t, 16> FP16_16;

template <typename StoreType, typename LongType, int Shift>
FixedPoint<StoreType, LongType, Shift>::FixedPoint(float x) { _value = (x * (StoreType(1) << Shift)); }

template <typename StoreType, typename LongType, int Shift>
FixedPoint<StoreType, LongType, Shift>::FixedPoint(double x) { _value = (x * (StoreType(1) << Shift)); }

template <typename StoreType, typename LongType, int Shift>
bool isnan(FixedPoint<StoreType, LongType, Shift>) { return false; }

template <typename StoreType, typename LongType, int Shift>
float sin(FixedPoint<StoreType, LongType, Shift> x) {return sin(float(x));}

template <typename StoreType, typename LongType, int Shift>
float cos(FixedPoint<StoreType, LongType, Shift> x) {return cos(float(x));}

template <typename StoreType, typename LongType, int Shift>
float sqrt(FixedPoint<StoreType, LongType, Shift> x) {return sqrt(float(x));}

template <typename StoreType, typename LongType, int Shift>
float atan2(
    FixedPoint<StoreType, LongType, Shift> y,
    FixedPoint<StoreType, LongType, Shift> x) {return atan2(float(y), float(x));}

#endif // DEVICE_FIXED_POINT_H
