

#pragma once


#include <iostream>
#include <regex>
#include <map>
#include <queue>
#include <unordered_map>

#include "clau_parser.h"

#include "smart_ptr.h"

//#include "ArrayMap.h"

#include "claujson.h"

using namespace std::literals;

template <class T, class T2>
using myMap = std::unordered_map<T, T2>;

enum FUNC {
	NONE = 0, TRUE, FALSE, FUNC_IS_INT, FUNC_IS_FLOAT, FUNC_GET_GLOBAL, FUNC_SET_GLOBAL, FUNC_MAKE_GLOBAL, FUNC_SPLIT, FUNC_CLEAR_GLOBAL,
	FUNC_REMOVE, FUNC_COUNT_GLOBAL,
	FUNC_SEARCH, FUNC_QUERY, FUNC_ASSIGN, FUNC_GET, FUNC_FIND, FUNC_WHILE, FUNC_RETURN_VALUE, FUNC_IS_END,
	FUNC_NOT, FUNC_NOT_EMPTY,
	FUNC_LOAD_DATA, FUNC_LOAD_JSON, FUNC_ENTER, FUNC_CALL, FUNC_NEXT, FUNC_RETURN, FUNC_COMP_RIGHT,
	FUNC_ADD, FUNC_GET_IDX, FUNC_GET_SIZE, FUNC_GET_NOW, FUNC_CLONE, FUNC_QUIT, FUNC_IF, FUNC_IS_ITEM,
	FUNC_IS_GROUP, FUNC_SET_IDX, FUNC_AND_ALL, FUNC_AND, FUNC_OR, FUNC_IS_QUOTED_STR, FUNC_COMP_LEFT,
	FUNC_SET_NAME, FUNC_GET_NAME, FUNC_GET_VALUE,
	FUNC_SET_VALUE, FUNC_REMOVE_QUOTED, CONSTANT, THEN, WHILE_END, IF_END, START_DIR, DIR, END_DIR, 
	FUNC_PRINT, 
	KEY, VALUE, SIZE // chk?
};
inline const char* func_to_str[FUNC::SIZE] = {
	"TRUE", "FALSE", 
	"IS_INT", "IS_FLOAT",
	"GET_GLOBAL", "SET_GLOBAL",
	"MAKE_GLOBAL",
	"SPLIT", "CLEAR_GLOBAL", "REMOVE", "COUNT_GLOBAL",
	"SEARCH", "QUERY",
	"ASSIGN",
	"GET",
	"FIND", "WHILE", "RETURN_VALUE", "IS_END", "NOT", 
	"NOT_EMPTY",  // for workspace.
	"LOAD_DATA", "LOAD_JSON",
	"ENTER", "CALL", "NEXT", "RETURN", "COMP_RIGHT",
	"ADD", "GET_IDX", "GET_SIZE", "GET_NOW", "CLONE", "QUIT", "IF", "IS_ITEM",
	"IS_GROUP", "SET_IDX", "AND_ALL", "AND", "OR", "IS_QUOTED_STR", "COMP_LEFT",
	"SET_NAME", "GET_NAME", "GET_VALUE",
	"SET_VALUE", "REMOVE_QUOTED", "CONSTANT", "THEN", "WHILE_END", "IF_END", 
	"START_DIR", "DIR", "END_DIR", "PRINT", 
	"NONE", "KEY", "VALUE"
};

class Workspace {
public:
	wiz::SmartPtr<clau_parser::Reader> reader;
public:
	Workspace(clau_parser::UserType* ut = nullptr) {
		if (reader) {
			*reader = clau_parser::Reader(ut);
		}
	}
};

namespace claujson {
	class Explorer {
	private:
		claujson::_Value* root;
		std::vector<std::pair<uint64_t, claujson::_Value*>> _stack;

	public:
		Explorer(claujson::_Value& value) {
			root = new claujson::_Value(claujson::Array::Make());
			
			root->as_array()->add_element(std::move(value));
			_stack.push_back({ 0, root });
		}
		
		Explorer() : root(nullptr) {
			//
		}
		~Explorer() {
			if (root) {
				root->as_array()->clear();
				delete root;
			}
		}	

	public:
		bool empty() const {
			return _stack.empty();
		}

	private:
		claujson::_Value* Now() {
			return _stack.back().second;
		}

		const claujson::_Value* Now() const {
			return _stack.back().second;
		}

	public:
		bool IsPrimitiveRoot() const {
			return root->as_array()->get_value_list(0).is_primitive();
		}

		uint64_t GetIdx() const {
			if (_stack.empty()) { return 0; }
			return _stack.back().first;
		}

		void SetIdx(const uint64_t idx) {
			if (_stack.empty()) { return; }
			_stack.back().first = idx;
		}

		uint64_t Size() const {
			return Now()->as_structured_ptr().size();
		}

		claujson::_Value& Get() {
			if (IsPrimitiveRoot()) {
				return root->as_array()->get_value_list(0);
			}
			return claujson::StructuredPtr(Now()->as_structured_ptr()).get_value_list(GetIdx());
		}

		const claujson::_Value& Get() const {
			if (IsPrimitiveRoot()) {
				return root->as_array()->get_value_list(0);
			}
			return claujson::StructuredPtr(Now()->as_structured_ptr()).get_value_list(GetIdx());
		}

		const claujson::_Value& GetKey() const {
			static claujson::_Value nkey(nullptr, false);
			if (!Now() || claujson::StructuredPtr(Now()->as_structured_ptr()).is_array()) {
				return nkey;
			}
			return claujson::StructuredPtr(Now()->as_structured_ptr()).get_const_key_list(GetIdx());
		}

		void ChangeKey(claujson::Value new_key) {
			if (Now()) {
				claujson::StructuredPtr(Now()->as_structured_ptr()).change_key(GetKey(), std::move(new_key));
			}
		}

		void Delete() {
			if (Now()) {
				claujson::StructuredPtr(Now()->as_structured_ptr()).erase(GetIdx(), true);
			}
		}

		void Enter() {
			if (Get().is_structured()) {
				auto* x = Now();
				uint64_t idx = GetIdx();
				_stack.push_back({ 0, nullptr });
				_stack.back().second = &x->as_structured_ptr().get_value_list(idx);
			}
		}

		void Quit() {
			if (_stack.empty()) {
				return;
			}
			_stack.pop_back();
		}

		// Group <- Array or Object!
		bool IsEndOfGroup() { // END Of GROUP?
			if (IsPrimitiveRoot()) {
				return true;
			}
			if (Now() == nullptr) {
				return true;
			}
			return GetIdx() >= claujson::StructuredPtr(Now()->as_structured_ptr()).get_data_size();
		}

		bool Next() {
			if (!IsEndOfGroup()) {
				SetIdx(GetIdx() + 1);
				return true;
			}
			return false;
		}

		// todo - goto using json pointer?, dir is STRING or UNSIGNED_INTEGER
		void Goto(const std::vector<claujson::_Value>& dir) {
			//
		}

		void Dump(std::ostream& out) {
			Dump(out, this);
		}
		// check.. cf) simdjson`s Dump?
		void Dump(std::ostream& out, Explorer* exp) {
			while (!IsEndOfGroup()) {
				if (GetKey().is_str()) {
					out << GetKey() << " : ";
				}
				if (Get().is_primitive()) {
					out << Get() << " ";
				}
				else {
					if (Get().is_array()) {
						out << " [ ";
					}
					else {
						out << " { ";
					}

					Enter();

					Dump(out, exp);

					Quit();

					if (Get().is_array()) {
						out << " ] \n";
					}
					else {
						out << " } \n";
					}
				}
				Next();
			}
		}
	};
}
enum class ValueType {
	none,
	end_of_container,
	end_of_document,
	container, // array or object
	key,
	value
};


inline std::ostream& operator<<(std::ostream& out, ValueType t) {
	switch (t) {
	case ValueType::none: {
		out << "NONE";
		break;
	}
	case ValueType::end_of_container: {
		out << "end of container";
		break;
	}
	case ValueType::end_of_document: {
		out << "end of document";
		break;
	}
	case ValueType::container: {
		out << "container";
		break;
	}
	case ValueType::key: {
		out << "key";
		break;
	}
	case ValueType::value: {
		out << "value";
		break;
	}
	}
	return out;
}

// for json
class WorkspaceJ {
public:
	wiz::SmartPtr<claujson::Explorer> reader;
public:
	WorkspaceJ() { }

	WorkspaceJ(claujson::_Value& json) {
		if (reader) {
			*reader = claujson::Explorer(json);
		}
	}
};

class Token {
public:
	FUNC func = FUNC::NONE;
	
	union {
		mutable long long int_val = 0;
		mutable long double float_val;
	};
	
	mutable std::string str_val;

	enum Type { INT = 1, FLOAT = 2, STRING = 4, FUNC_ = 8, USERTYPE = 16, WORKSPACE = 32, WORKSPACEJ = 64, BOOL = 128, NONE = 256 };
	mutable Type type = Type::NONE;

public:
	wiz::SmartPtr<clau_parser::UserType> ut_val = nullptr;
	
	Workspace workspace;
	WorkspaceJ workspacej;


	long long line = 0;


	Token(clau_parser::UserType* ut = nullptr) : workspace(ut) {
		//
	}

	const std::string& ToString() const {

		if (type & Type::STRING) {
			return str_val;
		}

		if (type & Type::INT) {
			str_val = std::to_string(int_val);
			type = static_cast<Type>(type | Type::STRING);
			return str_val;
		}

		if (type & Type::FLOAT) {
			str_val = std::to_string(float_val);
			type = static_cast<Type>(type | Type::STRING);
			return str_val;
		}


		if (type & Type::BOOL) {
			str_val = int_val ? "TRUE" : "FALSE";
			type = static_cast<Type>(type | Type::STRING);
			return str_val;
		}
		// throw error?

		return str_val;
	}
	long long ToInt() const {
		if (type & Type::INT) {
			return int_val;
		}

		if (type & Type::FLOAT) {
			return float_val;
		}

		if (type & Type::STRING) {
			int_val = std::stoll(str_val);
			type = static_cast<Type>(type | Type::INT);
			return int_val;
		}

		return 0;
	}
	long double ToFloat() const {
		if (type & Type::FLOAT) {
			return float_val;
		}

		if (type & Type::INT) {
			return int_val;
		}

		if (type & Type::STRING) {
			float_val = std::stold(str_val);
			type = static_cast<Type>(type | Type::FLOAT);
			return float_val;
		}

		return 0;
	}
	bool ToBool() const {
		if (type & Type::BOOL) {
			return int_val == 1;
		}

		if (type & Type::INT) {
			type = static_cast<Type>(type | Type::BOOL);
			return int_val;
		}

		if (type & Type::FLOAT) {
			type = static_cast<Type>(type | Type::BOOL);
			return float_val;
		}

		return false;
	}
	void SetString(const std::string& str) {
		str_val = str;
		type = Type::STRING;
	}
	void SetInt(long long x) {
		int_val = x;
		type = Type::INT;
	}
	void SetFloat(long double x) {
		float_val = x;
		type = Type::FLOAT;
	}
	void SetWorkspace(wiz::SmartPtr<clau_parser::Reader> ptr) {
		workspace.reader = ptr;
		type = Type::WORKSPACE;
	}
	void SetWorkspaceJ(wiz::SmartPtr<claujson::Explorer> ptr) {
		workspacej.reader = ptr;
		type = Type::WORKSPACEJ;
	}
	void SetBool(bool x) {
		int_val = x ? 1 : 0;
		type = Type::BOOL;
	}
	void SetFunc() {
		type = Type::FUNC_;
	}

	bool IsInt() const {
		if (type & Type::INT) {
			return true;
		}

		if (type & Type::STRING) {

			int state = 0;
			const auto& str = str_val;

			for (int i = 0; i < str.size(); ++i) {
				switch (state)
				{
				case 0:
					if ('+' == str[i] || '-' == str[i]) {
						state = 0;
					}
					else if (str[i] >= '0' && str[i] <= '9')
					{
						state = 1;
					}
					else return false;
					break;
				case 1:
					if (str[i] >= '0' && str[i] <= '9') {
						state = 1;
					}
					else return false;
				}
			}
			if (1 == state) {
				//type = static_cast<Type>(type | Type::INT);
				//int_val = std::stoll(str_val);
				return true;
			}
		}

		return false;
	}
	bool IsFloat() const {
		if (type & Type::FLOAT) {
			return true;
		}

		if (type & Type::STRING) {
			int state = 0;
			const auto& str = str_val;

			for (int i = 0; i < str.size(); ++i) {
				switch (state)
				{
				case 0:
					if ('+' == str[i] || '-' == str[i]) {
						state = 0;
					}
					else if (str[i] >= '0' && str[i] <= '9')
					{
						state = 1;
					}
					else { return false; }
					break;
				case 1:
					if (str[i] >= '0' && str[i] <= '9') {
						state = 1;
					}
					else if (str[i] == '.') {
						state = 2;
					}
					else { return false; }
					break;
				case 2:
					if (str[i] >= '0' && str[i] <= '9') { state = 3; }
					else { return false; }
					break;
				case 3:
					if (str[i] >= '0' && str[i] <= '9') { state = 3; }
					else { return false; }
					break;
				}
			}
			if (state == 3) {
				//type = static_cast<Type>(type | Type::FLOAT);
				//float_val = std::stold(str_val);
				return true;
			}
		}

		return false;
	}
	bool IsString() const {
		return type & Type::STRING;
	}
	bool IsBool() const {
		return type & Type::BOOL;
	}
	bool IsWorkspace() const {
		return type & Type::WORKSPACE;
	}
	bool IsWorkspaceJ() const {
		return type & Type::WORKSPACEJ;
	}
};


std::vector<Token> FindValue(clau_parser::UserType* ut, const std::string& str);

struct Event {
	//Workspace workspace; 
	//WorkspaceJ workspacej; // for one json file?
	std::string id;
	std::vector<int> event_data;
	long long now = 0;
	std::vector<Token> return_value;
	long long return_value_now;
	wiz::SmartPtr<std::vector<Token>> input; // ?
	myMap<std::string, Token> parameter; // myMap
	myMap<std::string, Token> local; // myMap
};


class VM {
private:
	class UtInfo {
	public:
		clau_parser::UserType* global;
		clau_parser::UserType* ut;
		clau_parser::UserType* dest;
		std::string dir;
		long long itCount = 0;
		long long utCount = 0;
		long long count = 0;
		
	public:
		UtInfo(clau_parser::UserType* global, clau_parser::UserType* ut, const std::string& dir, clau_parser::UserType* dest = nullptr, long long itCount = 0, long long utCount = 0)
			: global(global), ut(ut), itCount(itCount), utCount(utCount), count(0), dir(dir), dest(dest)
		{
			//
		}
	};

	// for $insert, $update, $delete : lint
	bool EqualFunc(clau_parser::UserType* global, const clau_parser::ItemType<std::string>& x,
		clau_parser::ItemType<std::string> y, long long x_idx, const std::string& dir, VM* vm);


	bool _InsertFunc(clau_parser::UserType* global, clau_parser::UserType* insert_ut, VM* vm);

	bool _RemoveFunc(clau_parser::UserType* global, clau_parser::UserType* insert_ut, VM* vm);


	bool _UpdateFunc(clau_parser::UserType* global, clau_parser::UserType* insert_ut, VM* vm);

	// starts with '@' -> insert target
	// else -> condition target.
	bool InsertFunc(clau_parser::UserType* global, clau_parser::UserType* insert_ut, VM* vm);

	bool RemoveFunc(clau_parser::UserType* global, clau_parser::UserType* insert_ut, VM* vm);

	bool UpdateFunc(clau_parser::UserType* global, clau_parser::UserType* insert_ut, VM* vm);

	bool SearchFunc(clau_parser::UserType* global, clau_parser::UserType* insert_ut, clau_parser::UserType* dest, VM* vm);


	std::pair<bool, std::vector<clau_parser::UserType*> > _Find(clau_parser::UserType* global, const std::string& _position) /// option, option_offset
	{
		std::string position = _position;

		if (!position.empty() && position[0] == '@') { position.erase(position.begin()); }

		std::vector<std::string> x;


		//wiz::Out << "string view is " << pos_sv << " ";
		std::vector<std::string> tokens = clau_parser::tokenize(position, '/');

		for (size_t i = 0; i < tokens.size(); ++i) {
			std::string temp = tokens[i];

			//wiz::Out << tokens[i] << " ";

			if (temp == ".") {
				continue;
			}

			if (x.empty()) {
				x.push_back(temp);
			}
			else if (x.back() != ".." && temp == "..") {
				x.pop_back();
			}
			else {
				x.push_back(temp);
			}
		}

		return __Find(std::move(x), global);
	}
private:
	// find userType! not itemList!,
	// this has bug
	/// /../x ok   ( /../../y ok)
	/// /x/../x/../x no
	std::pair<bool, std::vector< clau_parser::UserType*> > __Find(std::vector<std::string>&& tokens, clau_parser::UserType* global) /// option, option_offset
	{
		std::vector<clau_parser::UserType* > temp;
		int start = 0;

		if (tokens.empty()) { temp.push_back(global); return{ true, temp }; }
		if (tokens.size() == 1 && tokens[0] == ".") { temp.push_back(global); return{ true, temp }; }
		//if (position.size() == 1 && position[0] == "/./") { temp.push_back(global); return{ true, temp }; } // chk..
		//if (position.size() == 1 && position[0] == "/.") { temp.push_back(global); return{ true, temp }; }
		//if (position.size() == 1 && position[0] == "/") { temp.push_back(global); return{ true, temp }; }

		if (tokens.size() > 1 && tokens[0] == ".")
		{
			start = 1;
			//position = String::substring(position, 3);
		}

		std::list<std::pair<clau_parser::UserType*, int >> utDeck;
		std::pair<clau_parser::UserType*, int> utTemp;
		utTemp.first = global;
		utTemp.second = 0;
		std::vector<std::string> strVec;

		//wiz::Out << "position is " << position << "\t";
		for (int i = start; i < tokens.size(); ++i) {
			std::string strTemp = tokens[i];

			//wiz::Out << strTemp << " ";

			if (strTemp == "root" && i == 0) {
			}
			else {
				strVec.push_back(strTemp);
			}

			if ((strVec.size() >= 1) && (" " == strVec[strVec.size() - 1])) /// chk!!
			{
				strVec[strVec.size() - 1] = "";
			}
			else if ((strVec.size() >= 1) && ("_" == strVec[strVec.size() - 1]))
			{
				strVec[strVec.size() - 1] = "";
			}
		}

		// it has bug!
		{
			int count = 0;

			for (int i = 0; i < strVec.size(); ++i) {
				if (strVec[i] == "..") {
					count++;
				}
				else {
					break;
				}
			}

			std::reverse(strVec.begin(), strVec.end());

			for (int i = 0; i < count; ++i) {
				if (utTemp.first == nullptr) {
					return{ false, std::vector< clau_parser::UserType* >() };
				}
				utTemp.first = utTemp.first->GetParent();
				strVec.pop_back();
			}
			std::reverse(strVec.begin(), strVec.end());
		}
		//wiz::Out << "\n";

		utDeck.push_front(utTemp);

		bool exist = false;
		while (false == utDeck.empty()) {
			utTemp = utDeck.front();
			utDeck.pop_front();

			if (utTemp.second < strVec.size() && strVec[utTemp.second] == "$")
			{
				for (int j = utTemp.first->GetUserTypeListSize() - 1; j >= 0; --j) {
					clau_parser::UserType* x = utTemp.first->GetUserTypeList(j);
					utDeck.push_front(std::make_pair(x, utTemp.second + 1));
				}
			}
			else if (utTemp.second < strVec.size() && strVec[utTemp.second].starts_with("$.")) /// $."abc"
			{
				std::string rex_str = strVec[utTemp.second].substr(3, strVec[utTemp.second].size() - 4);
				std::regex rgx(rex_str.data());

				for (int j = utTemp.first->GetUserTypeListSize() - 1; j >= 0; --j) {
					if (std::regex_match((utTemp.first->GetUserTypeList(j)->GetName()), rgx)) {
						clau_parser::UserType* x = utTemp.first->GetUserTypeList(j);
						utDeck.push_front(std::make_pair(x, utTemp.second + 1));
					}
				}
			}
			else if (utTemp.second < strVec.size() &&
				(utTemp.first->GetUserTypeItem(strVec[utTemp.second]).empty() == false))
			{
				auto  x = utTemp.first->GetUserTypeItem(strVec[utTemp.second]);
				for (int j = x.size() - 1; j >= 0; --j) {
					utDeck.push_front(std::make_pair(x[j], utTemp.second + 1));
				}
			}

			if (utTemp.second == strVec.size()) {
				exist = true;
				temp.push_back(utTemp.first);
			}
		}
		if (false == exist) {
			return{ false, std::vector<clau_parser::UserType*>() };
		}
		return{ true, temp };
	}

	std::vector<Token> Find(clau_parser::UserType* ut, std::string str) {
		std::vector<Token> result;
		auto uts = _Find(ut, str);

		if (!uts.first) {
			return result;
		}

		for (long long i = 0; i < uts.second.size(); ++i) {
			Token _token;
			_token.SetWorkspace(new clau_parser::Reader(uts.second[i]));
			result.push_back(_token);
		}

		return result;
	}
public:


	std::vector<Token> Run(const std::string& id, clau_parser::UserType* global,
		const myMap<std::string, Token>& parameter = myMap<std::string, Token>());

	void Register(Event e) {
		_event_list.insert(std::make_pair(e.id, e));
	}

private:
	myMap<std::string, Event> _event_list;
};


void _MakeByteCode(clau_parser::UserType* ut, Event* e);

void Debug(const Event& e);

// need to exception processing.
Event MakeByteCode(clau_parser::UserType* ut);