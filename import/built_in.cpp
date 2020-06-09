#include <sstream>
#include <cmath>
#include <chrono>
#include <thread>
#include "built_in.h"

namespace built_in {
	/*OBJECT*/

	/**
	 * Whether the object equals the other object.
	 * @param other The object compared to
	 * @return Whether the objects have the same pointer value
	 */
	_ptr<Boolean> Object::Equals(const _ptr<Object> &other) const {
		return SameAs(other);
	}

	_ptr<Boolean> Object::NotEquals(const _ptr<Object> &other) const {
		return Equals(other)->Not();
	}

	_ptr<Text> Object::TextValue() const {
		std::stringstream ss;
		ss << this;
		return ClassName(_this_ptr(this))->Add(_make_ptr<Text>(ss.str()));
	}

	_ptr<Boolean> Object::SameAs(const _ptr<Object> &other) const {
		return _make_ptr<Boolean>(this == other.get());
	}

	/*TEXT*/
	_ptr<Text> Text::TextValue() const {
		return _copy_ptr(this);
	}

	_ptr<Number> Text::NumberValue() const {
		return _make_ptr<Number>(std::stod(_value));
	}

	_ptr<Text> Text::Range(const _ptr<Number> &low, const _ptr<Number> &high) const {
		return _make_ptr<Text>(_value.substr(low->_value - 1, high->_value - low->_value + 1));
	}

	_ptr<Text> Text::Get(const _ptr<Number> &position) const {
		return Range(position, position);
	}

	_ptr<Boolean> Text::Equals(const _ptr<Text> &other) const {
		return _make_ptr<Boolean>(_value == other->_value);
	}

	_ptr<Text> Text::Add(const _ptr<Text> &other) const {
		return _make_ptr<Text>(_value + other->_value);
	}

	_ptr<Text> Text::Multiply(const _ptr<Number> &other) const {
		std::string result;
		for (int i = 0; i < other->_value; i++)
			result += _value;
		return _make_ptr<Text>(result);
	}

	_ptr<Boolean> Text::GreaterThan(const _ptr<Text> &other) const {
		return _make_ptr<Boolean>(_value.compare(other->_value) > 0);
	}

	_ptr<Boolean> Text::LessThan(const _ptr<Text> &other) const {
		return _make_ptr<Boolean>(_value.compare(other->_value) < 0);
	}

	_ptr<Boolean> Text::BooleanValue() const {
		return _make_ptr<Boolean>(!_value.empty());
	}


	Number::Number(double value) : _value(value) {
		double intPart;
		is_integer = new Boolean(std::modf(value, &intPart) == 0.0);
	}

	_ptr<Text> Number::TextValue() const {
		return _make_ptr<Text>(std::to_string(_value));
	}

	_ptr<Boolean> Number::Equals(const _ptr<Number> &other) const {
		return _make_ptr<Boolean>(_value == other->_value);
	}

	_ptr<Boolean> Number::GreaterThan(const _ptr<Number> &other) const {
		return _make_ptr<Boolean>(_value >= other->_value);
	}

	_ptr<Boolean> Number::LessThan(const _ptr<Number> &other) const {
		return _make_ptr<Boolean>(_value <= other->_value);
	}

	_ptr<Number> Number::Add(const _ptr<Number> &other) const {
		return _make_ptr<Number>(_value + other->_value);
	}

	_ptr<Number> Number::Subtract(const _ptr<Number> &other) const {
		return _make_ptr<Number>(_value - other->_value);
	}

	_ptr<Number> Number::Multiply(const _ptr<Number> &other) const {
		return _make_ptr<Number>(_value * other->_value);
	}

	_ptr<Number> Number::Divide(const _ptr<Number> &other) const {
		return _make_ptr<Number>(_value / other->_value);
	}

	_ptr<Number> Number::Modulus(const _ptr<Number> &other) const {
		if (is_integer && other->is_integer)
			return _make_ptr<Number>((int) _value % (int) other->_value);
		else {
			std::cout << "Type Error: Modulus can only accept integer values.";
			exit(1);
		}
	}

	_ptr<Boolean> Number::BooleanValue() const {
		return _make_ptr<Boolean>(_value != 0);
	}

	_ptr<Number> Number::NumberValue() const {
		return _copy_ptr(this);
	}

	/*BOOLEAN*/

	Boolean::Boolean(bool value) {
		this->_value = value;
	}

	_ptr<Text> Boolean::TextValue() const {
		return _make_ptr<Text>(_value ? "true" : "false");
	}

	_ptr<Number> Boolean::NumberValue() const {
		return _make_ptr<Number>(_value);
	}

	_ptr<Boolean> Boolean::Equals(const _ptr<Boolean> &other) const {
		return _make_ptr<Boolean>(_value == other->_value);
	}

	_ptr<Boolean> Boolean::And(const _ptr<Boolean> &other) const {
		return _make_ptr<Boolean>(_value && other->_value);
	}

	_ptr<Boolean> Boolean::Or(const _ptr<Boolean> &other) const {
		return _make_ptr<Boolean>(_value || other->_value);
	}

	_ptr<Boolean> Boolean::Xor(const _ptr<Boolean> &other) const {
		return NotEquals(other);
	}

	_ptr<Boolean> Boolean::Not() const {
		return _make_ptr<Boolean>(!_value);
	}

	_ptr<Boolean> Boolean::BooleanValue() const {
		return _copy_ptr(this);
	}

	/*METHODS*/

	_ptr<Text> ClassName(const _ptr<Object> &object) {
		return _make_ptr<Text>(typeid(object).name());
	}

	_ptr<Boolean> If(const _ptr<Boolean> &condition, const _ptr<Block<>> &then) {
		if (condition->_value) {
			then->Run();
			return _make_ptr<Boolean>(true);
		}
		return _make_ptr<Boolean>(false);
	}

	void SaySameLine(const _ptr<Object> &value) {
		_ptr<Text> txt = value->TextValue();
		std::cout << txt->_value << std::flush;
	}

	void Say(const _ptr<Object> &value) {
		_ptr<Text> txt = value->TextValue();
		std::cout << txt->_value << std::endl;
	}

	_ptr<Text> Ask(const _ptr<Object> &question) {
		SaySameLine(question);
		std::string answer;
		getline(std::cin, answer);
		return _make_ptr<Text>(answer);
	}

	void Wait(const _ptr<Number> &seconds) {
		std::this_thread::sleep_for(std::chrono::milliseconds((int) (seconds->_value * 1000)));
	}

	void Repeat(const _ptr<Number> &times, const _ptr<Block<void>> &block) {
		for (int i = 0; i < times->_value; i++)
			block->Run();
	}

	void While(std::function<_ptr<Boolean>()> condition, std::function<_ptr<Block<>>()> blk) {
		If(condition(), _make_ptr<Block<>>([&]() {
			blk()->Run();
			While(condition, blk);
		}));
	}
}