#ifndef __BISH_TYPE_H__
#define __BISH_TYPE_H__

namespace Bish {

class Type {
private:
    typedef enum {
        UndefinedTy, IntegerTy, FractionalTy, StringTy, BooleanTy
    } InternalType;
    InternalType type;
    bool array;
    Type(InternalType ty) : type(ty), array(false) {}
public:
    static Type Undef() { return Type(UndefinedTy); }
    static Type Integer() { return Type(IntegerTy); }
    static Type Fractional() { return Type(FractionalTy); }
    static Type String() { return Type(StringTy); }
    static Type Boolean() { return Type(BooleanTy); }

    bool defined() const { return type != UndefinedTy; }
    bool undef() const { return type == UndefinedTy; }
    bool integer() const { return type == IntegerTy; }
    bool fractional() const { return type == FractionalTy; }
    bool string() const { return type == StringTy; }
    bool boolean() const { return type == BooleanTy; }

    bool operator==(const Type &b) const {
        return type == b.type && array == b.array;
    }
};

}

#endif
