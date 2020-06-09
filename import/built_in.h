//rt While

#ifndef SIMPLETHON_BUILT_IN_H
#define SIMPLETHON_BUILT_IN_H

#include <functional>
#include <iostream>
#include <cstdlib>

namespace built_in {
	class Object;

	class Text;

	class Number;

	template<typename RT=void, typename... AT>
	class Block;

	class Boolean;

	class _counter {
	public:
		_counter() : count(1) {};

		_counter(const _counter &) = delete;

		_counter &operator=(const _counter &) = delete;

		~_counter() = default;

		void reset() { count = 1; }

		inline unsigned int get() const { return count; }

		inline void operator++() { count++; }

		inline void operator++(int) { count++; }

		inline void operator--() { count--; }

		inline void operator--(int) { count--; }

	private:
		unsigned int count;
	};

	template<typename T>
	class _ptr {
	public:
		_ptr() : ptr(nullptr), counter(nullptr) {}

		explicit _ptr(T *val) : ptr(val) {
			counter = new _counter();
		}

		_ptr(T *val, _counter *count) : ptr(val), counter(count) {}

		_ptr(const _ptr<T> &sp) : ptr(sp.get()) {
			counter = sp.counter;
			counter++;
		}

		_ptr(T *val, bool init) : _ptr(val) { if (init) counter++; }

		inline unsigned int count() const { return counter->get(); }

		template<typename U>
		operator _ptr<U>() const {
			if (std::is_base_of<U, T>())
				return _ptr<U>((U *) ptr, counter);
			else {
				std::cout << "Error: Cannot cast type " << typeid(T).name() << " to " << typeid(U).name();
				exit(1);
			}
		}

		~_ptr() {
			if ((--counter)->get() == 0) {
				delete counter;
				delete this->get();
			}
		}

		inline T *get() const { return ptr; }

		inline T &operator*() const { return *ptr; }

		inline T *operator->() const { return ptr; }

		operator T() { return *ptr; }

	private:
		T *ptr;
		_counter *counter{};
	};

	template<typename T, typename... AT>
	_ptr<T> _make_ptr(AT... args) { return _ptr<T>(new T(args...)); }

	template<typename T>
	_ptr<T> _copy_ptr(T *arg) { return _make_ptr<T>(*arg); }

	template<typename T>
	_ptr<T> _this_ptr(T *arg) { return _ptr<T>(arg, true); }

	class Object {
	public:
		virtual _ptr<Text> TextValue() const;

		virtual _ptr<Boolean> Equals(const _ptr<Object> &other) const;

		virtual _ptr<Boolean> NotEquals(const _ptr<Object> &other) const;

		_ptr<Boolean> SameAs(const _ptr<Object> &other) const;
	};

	class Number : public Object {
	public:
		double _value;
		Boolean *is_integer;

		Number(double value);

		_ptr<Text> TextValue() const override;

		_ptr<Boolean> BooleanValue() const;

		_ptr<Number> NumberValue() const;

		_ptr<Boolean> Equals(const _ptr<Number> &other) const;

		_ptr<Boolean> GreaterThan(const _ptr<Number> &other) const;

		_ptr<Boolean> LessThan(const _ptr<Number> &other) const;

		_ptr<Number> Add(const _ptr<Number> &other) const;

		_ptr<Number> Subtract(const _ptr<Number> &other) const;

		_ptr<Number> Multiply(const _ptr<Number> &other) const;

		_ptr<Number> Divide(const _ptr<Number> &other) const;

		_ptr<Number> Modulus(const _ptr<Number> &other) const;
	};

	class Boolean : public Object {
	public:
		bool _value;

		Boolean(bool value);

		_ptr<Boolean> BooleanValue() const;

		_ptr<Text> TextValue() const override;

		_ptr<Number> NumberValue() const;

		_ptr<Boolean> Equals(const _ptr<Boolean> &other) const;

		_ptr<Boolean> And(const _ptr<Boolean> &other) const;

		_ptr<Boolean> Or(const _ptr<Boolean> &other) const;

		_ptr<Boolean> Xor(const _ptr<Boolean> &other) const;

		_ptr<Boolean> Not() const;
	};

	class Text : public Object {
	public:
		std::string _value;

		Text(std::string value) : _value(std::move(value)) {}

		_ptr<Text> TextValue() const override;

		_ptr<Boolean> BooleanValue() const;

		_ptr<Number> NumberValue() const;

		_ptr<Text> Range(const _ptr<Number> &low, const _ptr<Number> &high) const;

		_ptr<Text> Get(const _ptr<Number> &position) const;

		_ptr<Boolean> Equals(const _ptr<Text> &other) const;

		_ptr<Text> Add(const _ptr<Text> &other) const;

		_ptr<Text> Multiply(const _ptr<Number> &other) const;

		_ptr<Boolean> GreaterThan(const _ptr<Text> &other) const;

		_ptr<Boolean> LessThan(const _ptr<Text> &other) const;
	};

	template<typename RT, typename... AT>
	class Block : public Object {
	public:
		std::function<RT(AT...)> _func;

		Block(std::function<RT(AT...)> func) {
			this->_func = func;
		}

		RT Run(AT... args) const {
			return _func(args...);
		}

		operator RT() const {
			return Run();
		}
	};

	_ptr<Boolean> If(const _ptr<Boolean> &condition, const _ptr<Block<>> &then);

	void Say(const _ptr<Object> &value);

	void SaySameLine(const _ptr<Object> &value);

	_ptr<Text> Ask(const _ptr<Object> &question);

	void Wait(const _ptr<Number> &seconds);

	void Repeat(const _ptr<Number> &times, const _ptr<Block<>> &block);

	void Exit();

	_ptr<Text> ClassName(const _ptr<Object> &object);

}

#endif //SIMPLETHON_BUILT_IN_H
