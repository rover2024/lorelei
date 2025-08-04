#ifndef INTORSTRING_H
#define INTORSTRING_H

#include <string>

namespace TLC {

    class IntOrString {
    public:
        enum Type {
            Int,
            String,
        };

        inline IntOrString() : IntOrString(0) {
        }

        /// Construct from an integer.
        inline IntOrString(int i) {
            _type = Int;
            _storage.i = i;
        }

        /// Construct from a string.
        inline IntOrString(std::string s) {
            _type = String;
            new (&_storage.s) std::string(std::move(s));
        }

        /// Destructor.
        inline ~IntOrString() {
            destroy();
        }

        /// Copy constructor.
        inline IntOrString(const IntOrString &RHS) {
            copyConstruct(RHS);
        }

        /// Move constructor.
        inline IntOrString(IntOrString &&RHS) {
            moveConstruct(RHS);
        }

        /// Copy assignment operator.
        inline IntOrString &operator=(const IntOrString &RHS) {
            if (this != &RHS) {
                destroy();
                copyConstruct(RHS);
            }
            return *this;
        }

        /// Move assignment operator.
        inline IntOrString &operator=(IntOrString &&RHS) {
            if (this != &RHS) {
                destroy();
                moveConstruct(RHS);
            }
            return *this;
        }

        static IntOrString fromAnyString(const std::string &s);

    public:
        inline Type type() const {
            return static_cast<Type>(_type);
        }
        inline bool isInt() const {
            return _type == Int;
        }
        inline bool isString() const {
            return _type == String;
        }
        inline int toInt() const {
            return _storage.i;
        }
        inline const std::string &toString() const {
            return _storage.s;
        }
        inline std::string &toString() {
            return _storage.s;
        }

    protected:
        inline void copyConstruct(const IntOrString &RHS) {
            _type = RHS._type;
            if (_type == Int) {
                _storage.i = RHS._storage.i;
            } else if (_type == String) {
                new (&_storage.s) std::string(RHS._storage.s);
            }
        }

        inline void moveConstruct(IntOrString &RHS) {
            _type = RHS._type;
            if (_type == Int) {
                _storage.i = RHS._storage.i;
            } else if (_type == String) {
                new (&_storage.s) std::string(std::move(RHS._storage.s));
            }
        }

        inline void destroy() {
            if (_type == String) {
                _storage.s.~basic_string();
            }
        }

        union Storage {
            std::string s;
            int i;
            Storage() {};
            ~Storage() {};
        } _storage;
        int _type;
    };

}

#endif // INTORSTRING_H
