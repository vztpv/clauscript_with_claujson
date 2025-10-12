
#include "vm.h"

inline bool empty(std::vector<Event>& stack) {
	return stack.empty();
}
inline bool empty(int64_t stack_count) {
	return stack_count <= 0;
}
inline Event& back(std::vector<Event>& stack, int64_t& stack_count) {
	return stack[stack_count - 1];
}
inline void pop_back(std::vector<Event>& stack, int64_t& stack_count) {
	stack_count--;
}
inline void push_back(std::vector<Event>& stack, int64_t& stack_count, Event&& e) {
	if (stack_count < stack.size()) {
		stack[stack_count] = std::move(e);
	}
	else {
		stack.push_back(std::move(e));
	}
	stack_count++;
}

inline bool empty(std::vector<Token>& stack) {
	return stack.empty();
}
inline Token& back(std::vector<Token>& stack, int64_t& stack_count) {
	return stack[stack_count - 1];
}
inline void pop_back(std::vector<Token>& stack, int64_t& stack_count) {
	stack_count--;
}
inline void push_back(std::vector<Token>& stack, int64_t& stack_count, Token&& t) {
	if (stack_count < stack.size()) {
		stack[stack_count] = std::move(t);
	}
	else {
		stack.push_back(std::move(t));
	}
	stack_count++;
}


std::vector<Token> VM::Run(std::string str_id, clau_parser::UserType* global,
	const myMap<const char*, Token>& parameter) {
	
	claujson::parser p;
	claujson::Document d;

	int64_t id = this->_event_map[str_id.c_str()];
	Event& main = _event_list[id];
	main.code = &_event_code_list[id];

	//main.parameter = parameter;
	for (auto& x : parameter) {
		main.parameter.insert({ x.first, x.second.clone() });
	}
	std::vector<Token> token_stack;
	int64_t token_count = 0;
	std::vector<Event> _stack;
	int64_t _stack_count = 0;
	
	myMap<std::string, std::vector<Token>> global_var;

	token_stack.reserve(1024);
	_stack.reserve(1024);
	_stack.push_back(main.clone());
	_stack_count++;

	int count = 0;
	std::string dir = "";

	std::vector<Token> return_value;

    /*
    // Computed Goto를 사용한 최적화 (GCC/Clang 전용)
    // 
        // Dispatch table - 각 opcode에 대한 레이블 주소
       // Computed Goto를 사용한 최적화 (GCC/Clang 전용)
// -fno-threaded-jumps 컴파일 옵션 필요 없음
        // Dispatch table - enum FUNC 순서에 맞춰 정렬
        static const void* dispatch_table[] = {
            && OP_NONE,              // 0

            && OP_TRUE,              // 1
            && OP_FALSE,             // 2
            && OP_FUNC_IS_INT,       // 3
            && OP_FUNC_IS_FLOAT,     // 4
            && OP_FUNC_GET_GLOBAL,   // 5
            && OP_FUNC_SET_GLOBAL,   // 6
            && OP_FUNC_MAKE_GLOBAL,  // 7
            && OP_FUNC_SPLIT,        // 8
            && OP_FUNC_CLEAR_GLOBAL, // 9
            && OP_FUNC_REMOVE,       // 10
            && OP_FUNC_COUNT_GLOBAL, // 11
            && OP_FUNC_SEARCH,       // 12
            && OP_FUNC_QUERY,        // 13
            && OP_FUNC_ASSIGN,       // 14
            && OP_FUNC_GET,          // 15
            && OP_FUNC_FIND,         // 16
            && OP_FUNC_WHILE,        // 17
            && OP_FUNC_RETURN_VALUE, // 18
            && OP_FUNC_IS_END,       // 19
            && OP_FUNC_NOT,          // 20
            && OP_FUNC_NOT_EMPTY,    // 21
            && OP_FUNC_LOAD_DATA,    // 22
            && OP_FUNC_LOAD_JSON,    // 23
            && OP_FUNC_ENTER,        // 24
            && OP_FUNC_CALL,         // 25
            && OP_FUNC_NEXT,         // 26
            && OP_FUNC_RETURN,       // 27
            && OP_FUNC_COMP_RIGHT,   // 28
            && OP_FUNC_ADD,          // 29
            && OP_FUNC_GET_IDX,      // 30
            && OP_FUNC_GET_SIZE,     // 31
            && OP_FUNC_GET_NOW,      // 32
            && OP_FUNC_CLONE,        // 33
            && OP_FUNC_QUIT,         // 34
            && OP_FUNC_IF,           // 35
            && OP_FUNC_IS_ITEM,      // 36
            && OP_FUNC_IS_GROUP,     // 37
            && OP_FUNC_SET_IDX,      // 38
            && OP_FUNC_AND_ALL,      // 39
            && OP_FUNC_AND,          // 40
            && OP_FUNC_OR,           // 41
            && OP_FUNC_IS_QUOTED_STR,// 42
            && OP_FUNC_COMP_LEFT,    // 43
            && OP_FUNC_SET_NAME,     // 44
            && OP_FUNC_GET_NAME,     // 45
            && OP_FUNC_GET_VALUE,    // 46
            && OP_FUNC_SET_VALUE,    // 47
            && OP_FUNC_REMOVE_QUOTED,// 48
            && OP_CONSTANT,          // 49
            && OP_THEN,              // 50
            && OP_WHILE_END,         // 51
            && OP_IF_END,            // 52
            && OP_START_DIR,         // 53
            && OP_DIR,               // 54
            && OP_END_DIR,           // 55
            && OP_FUNC_PRINT,        // 56

            && OP_KEY,               // 57
            && OP_VALUE,             // 58
            && OP_SIZE,              // 59
        };
        while (!empty(_stack_count)) {
            auto& x = back(_stack, _stack_count);
            count++;

            // Computed goto - 직접 점프
            goto* dispatch_table[x.code->event_data[x.now]];

        OP_NONE:
        OP_KEY:
        OP_VALUE:
        OP_SIZE:
            x.now++;
            continue;

        OP_TRUE:
            {
                Token token;
                token.SetBool(true);
                push_back(token_stack, token_count, std::move(token));
            }
            x.now++;
            continue;

        OP_FALSE:
            {
                Token token;
                token.SetBool(false);
                push_back(token_stack, token_count, std::move(token));
            }
            x.now++;
            continue;

        OP_FUNC_IS_INT:
            {
                auto token = std::move(back(token_stack, token_count));
                pop_back(token_stack, token_count);
                token.SetBool(token.IsInt());
                push_back(token_stack, token_count, std::move(token));
            }
            x.now++;
            continue;

        OP_FUNC_IS_FLOAT:
            {
                auto token = std::move(back(token_stack, token_count));
                pop_back(token_stack, token_count);
                token.SetBool(token.IsFloat());
                push_back(token_stack, token_count, std::move(token));
            }
            x.now++;
            continue;

        OP_FUNC_SPLIT:
            {
                auto divider = std::move(back(token_stack, token_count));
                pop_back(token_stack, token_count);
                char ch = divider.ToString()[0];

                auto dir = std::move(back(token_stack, token_count));
                pop_back(token_stack, token_count);
                auto& dest = global_var[dir.ToString()];

                auto str = back(token_stack, token_count).ToString();
                pop_back(token_stack, token_count);

                auto arr = clau_parser::tokenize(str, ch);

                for (const auto& _ : arr) {
                    Token token;
                    token.SetString(_);
                    dest.push_back(std::move(token));
                }
            }
            x.now++;
            continue;

        OP_FUNC_GET_GLOBAL:
            {
                auto idx = back(token_stack, token_count).ToInt();
                pop_back(token_stack, token_count);

                auto dir = back(token_stack, token_count).ToCString();
                pop_back(token_stack, token_count);

                push_back(token_stack, token_count, global_var[dir][idx].clone());
            }
            x.now++;
            continue;

        OP_FUNC_SET_GLOBAL:
            {
                auto value = std::move(back(token_stack, token_count));
                pop_back(token_stack, token_count);
                auto idx = back(token_stack, token_count).ToInt();
                pop_back(token_stack, token_count);
                auto dir = back(token_stack, token_count).ToString();
                pop_back(token_stack, token_count);

                global_var[dir][idx] = std::move(value);
            }
            x.now++;
            continue;

        OP_FUNC_MAKE_GLOBAL:
            {
                auto init = std::move(back(token_stack, token_count));
                pop_back(token_stack, token_count);

                auto dir = std::move(back(token_stack, token_count));
                pop_back(token_stack, token_count);

                global_var[dir.ToString()].push_back(std::move(init));
            }
            x.now++;
            continue;

        OP_FUNC_CLEAR_GLOBAL:
            {
                auto dir = std::move(back(token_stack, token_count));
                pop_back(token_stack, token_count);

                auto& dest = global_var[dir.ToString()];
                dest.clear();
            }
            x.now++;
            continue;

        OP_FUNC_REMOVE:
            {
                auto dir = std::move(back(token_stack, token_count));
                pop_back(token_stack, token_count);

                auto arr = Find(global, dir.ToString());

                for (int i = 0; i < arr.size(); ++i) {
                    auto ut = arr[i].workspace.GetReader()->GetUT();
                    auto parent = ut->GetParent();

                    for (int j = 0; j < parent->GetUserTypeListSize(); ++j) {
                        if (parent->GetUserTypeList(j) == ut) {
                            parent->RemoveUserTypeList(j);
                        }
                    }
                }
            }
            x.now++;
            continue;

        OP_FUNC_COUNT_GLOBAL:
            {
                auto dir = std::move(back(token_stack, token_count));
                pop_back(token_stack, token_count);

                const auto& arr = global_var[dir.ToString()];
                Token token;
                token.SetInt(arr.size());
                push_back(token_stack, token_count, std::move(token));
            }
            x.now++;
            continue;

        OP_FUNC_SEARCH:
            {
                x.now++;

                auto& ut = (*x.code->input)[x.code->event_data[x.now]];
                auto to = std::move(back(token_stack, token_count));
                pop_back(token_stack, token_count);
                auto dir = std::move(back(token_stack, token_count));
                pop_back(token_stack, token_count);

                auto& workspace = this->Find(global, dir.ToString())[0].workspace;
                auto& dest = this->Find(global, to.ToString())[0].workspace;

                for (int i = 2; i < ut.ut_val->GetUserTypeListSize(); ++i) {
                    this->SearchFunc(workspace.GetReader()->GetUT(), ut.ut_val->GetUserTypeList(i), dest.GetReader()->GetUT(), this);
                }
            }
            x.now++;
            continue;

        OP_FUNC_QUERY:
            {
                x.now++;

                auto& ut = (*x.code->input)[x.code->event_data[x.now]];
                auto dir = std::move(back(token_stack, token_count));
                pop_back(token_stack, token_count);

                auto& workspace = this->Find(global, dir.ToString())[0].workspace;

                for (int i = 1; i < ut.ut_val->GetUserTypeListSize(); ++i) {
                    if (ut.ut_val->GetUserTypeList(i)->GetName() == "$insert"sv) {
                        this->InsertFunc(workspace.GetReader()->GetUT(), ut.ut_val->GetUserTypeList(i), this);
                    }
                    else if (ut.ut_val->GetUserTypeList(i)->GetName() == "$update"sv) {
                        this->UpdateFunc(workspace.GetReader()->GetUT(), ut.ut_val->GetUserTypeList(i), this);
                    }
                    else if (ut.ut_val->GetUserTypeList(i)->GetName() == "$delete"sv) {
                        this->RemoveFunc(workspace.GetReader()->GetUT(), ut.ut_val->GetUserTypeList(i), this);
                    }
                }
            }
            x.now++;
            continue;

        OP_FUNC_GET:
            {
                auto token = std::move(back(token_stack, token_count));
                pop_back(token_stack, token_count);

                if (token.IsLocal()) {
                    token.ConvertLocal();
                    push_back(token_stack, token_count, x.local[token.ToCString()].clone());
                }
                else {
                    auto value = FindValue(global, token.ToString());
                    push_back(token_stack, token_count, std::move(value[0]));
                }
            }
            x.now++;
            continue;

        OP_FUNC_ASSIGN:
            {
                auto value = std::move(back(token_stack, token_count));
                pop_back(token_stack, token_count);

                auto name = std::move(back(token_stack, token_count));
                pop_back(token_stack, token_count);

                if (name.IsLocal()) {
                    x.local[name.ToCString()] = std::move(value);
                }
                else {
                    auto idx = name.ToString().find_last_of('/');
                    if (idx != std::string::npos) {
                        auto dir = name.ToString().substr(0, idx);
                        auto _name = name.ToString().substr(idx + 1);
                        auto ut = Find(global, dir);

                        for (int i = 0; i < ut.size(); ++i) {
                            auto arr = ut[i].workspace.GetReader()->GetUT()->GetItemIdx(_name);

                            for (int j = 0; j < arr.size(); ++j) {
                                ut[i].workspace.GetReader()->GetUT()->GetItemList(arr[j]).Set(0, value.ToString());
                            }
                        }
                    }
                }
            }
            x.now++;
            continue;

        OP_START_DIR:
            count = 0;
            dir = "/";
            x.now++;
            continue;

        OP_DIR:
            {
                auto& str = back(token_stack, token_count);

                if (str.IsParameter()) {
                    str.ConvertParameter();
                    Token& token = x.parameter[str.ToCString()];
                    dir += token.ToString();
                }
                else if (str.IsLocal()) {
                    str.ConvertLocal();
                    Token& token = x.local[str.ToCString()];
                    dir += token.ToString();
                }
                else {
                    dir += back(token_stack, token_count).ToString();
                }

                pop_back(token_stack, token_count);
                dir += "/";
            }
            x.now++;
            continue;

        OP_END_DIR:
            {
                Token token;

                if (dir.back() == '/') {
                    token.SetString(dir.substr(0, dir.size() - 1));
                }
                else {
                    token.SetString(dir);
                }
                push_back(token_stack, token_count, std::move(token));

                dir = "";
                count = 0;
            }
            x.now++;
            continue;

        OP_FUNC_REMOVE_QUOTED:
            {
                auto str = back(token_stack, token_count).ToString();

                if (str.size() >= 2) {
                    str = str.substr(1, str.size() - 2);
                }
                pop_back(token_stack, token_count);

                Token temp;
                temp.SetString(str);
                push_back(token_stack, token_count, std::move(temp));
            }
            x.now++;
            continue;

        OP_FUNC_IS_QUOTED_STR:
            {
                const auto& str = back(token_stack, token_count).ToString();
                bool chk = str.size() >= 2 && (str)[0] == str.back() && str.back() == '\"';

                pop_back(token_stack, token_count);

                Token temp;
                temp.SetBool(chk);

                push_back(token_stack, token_count, std::move(temp));
            }
            x.now++;
            continue;

        OP_FUNC_RETURN:
            x.now++;
            pop_back(_stack, _stack_count);
            continue;

        OP_CONSTANT:
            x.now++;
            {
                const auto& value = (*x.code->input)[x.code->event_data[x.now]];

                if (value.IsParameter()) {
                    value.ConvertParameter();
                    auto param = value.ToCString();
                    push_back(token_stack, token_count, x.parameter[param].clone());
                }
                else {
                    push_back(token_stack, token_count, value.clone());
                }
            }
            x.now++;
            continue;

        OP_FUNC_ADD:
            {
                auto a = std::move(back(token_stack, token_count));
                pop_back(token_stack, token_count);
                auto b = std::move(back(token_stack, token_count));
                pop_back(token_stack, token_count);

                Token token;

                if (a.IsFloat() && b.IsFloat()) {
                    token.SetFloat(a.ToFloat() + b.ToFloat());
                }
                else if (a.IsInt() && b.IsInt()) {
                    token.SetInt(a.ToInt() + b.ToInt());
                }

                push_back(token_stack, token_count, std::move(token));
            }
            x.now++;
            continue;

        OP_FUNC_CALL:
            x.now++;
            {
                auto param_count = x.code->event_data[x.now];
                x.now++;

                Event e;

                for (int i = 0; i < param_count; ++i) {
                    auto value = std::move(back(token_stack, token_count));
                    pop_back(token_stack, token_count);
                    auto name = std::move(back(token_stack, token_count));
                    pop_back(token_stack, token_count);

                    if (name.ToCString() == "id"sv) {
                        if (value.IsInt()) {
                            e.code = &_event_code_list[value.ToInt()];
                        }
                        else {
                            e.code = &_event_code_list[_event_map[value.ToCString()]];
                        }
                        e.now = 0;
                        e.return_value_now = 0;
                        continue;
                    }

                    e.parameter[name.ToCString()] = std::move(value);
                }
                if (!e.code) {
                    exit(-1);
                }
                push_back(_stack, _stack_count, std::move(e));
            }
            continue;

        OP_FUNC_COMP_LEFT:
            {
                auto a = std::move(back(token_stack, token_count));
                pop_back(token_stack, token_count);
                auto b = std::move(back(token_stack, token_count));
                pop_back(token_stack, token_count);

                Token token;

                if (a.IsFloat() && b.IsFloat()) {
                    token.SetBool(b.ToFloat() > a.ToFloat());
                }
                else if (a.IsInt() && b.IsInt()) {
                    token.SetBool(b.ToInt() > a.ToInt());
                }
                else if (a.IsString() && b.IsString()) {
                    token.SetBool(b.ToString() > a.ToString());
                }

                push_back(token_stack, token_count, std::move(token));
            }
            x.now++;
            continue;

        OP_FUNC_COMP_RIGHT:
            {
                auto b = std::move(back(token_stack, token_count));
                pop_back(token_stack, token_count);
                auto a = std::move(back(token_stack, token_count));
                pop_back(token_stack, token_count);

                Token token;

                if (a.IsFloat() && b.IsFloat()) {
                    token.SetBool(a.ToFloat() < b.ToFloat());
                }
                else if (a.IsInt() && b.IsInt()) {
                    token.SetBool(a.ToInt() < b.ToInt());
                }
                else if (a.IsString() && b.IsString()) {
                    token.SetBool(a.ToString() < b.ToString());
                }

                push_back(token_stack, token_count, std::move(token));
            }
            x.now++;
            continue;

        OP_FUNC_FIND:
            {
                auto a = std::move(back(token_stack, token_count));
                pop_back(token_stack, token_count);

                x.return_value = Find(global, a.ToString());
                x.return_value_now = 0;
            }
            x.now++;
            continue;

        OP_FUNC_RETURN_VALUE:
            {
                if (!x.return_value.empty()) {
                    push_back(token_stack, token_count, x.return_value[0].clone());
                }
            }
            x.now++;
            continue;

        OP_FUNC_NEXT:
            x.return_value_now++;
            x.now++;
            continue;

        OP_FUNC_LOAD_DATA:
            {
                std::string fileName = back(token_stack, token_count).ToString();
                fileName = fileName.substr(1, fileName.size() - 2);
                pop_back(token_stack, token_count);

                clau_parser::UserType* dir = back(token_stack, token_count).workspace.GetReader()->GetUT();
                pop_back(token_stack, token_count);

                clau_parser::LoadData::LoadDataFromFile(fileName, *dir, 0, 0);
            }
            x.now++;
            continue;

        OP_FUNC_LOAD_JSON:
            {
                std::string fileName = back(token_stack, token_count).ToString();
                fileName = fileName.substr(1, fileName.size() - 2);
                pop_back(token_stack, token_count);

                p.parse(fileName, d, 0);

                x.return_value.clear();

                Token temp;
                temp.SetWorkspaceJ(new claujson::Explorer(d.Get()));
                x.return_value.push_back(std::move(temp));
                x.return_value_now = 0;
            }
            x.now++;
            continue;

        OP_FUNC_ENTER:
            if (back(token_stack, token_count).IsWorkspace()) {
                back(token_stack, token_count).workspace.GetReader()->Enter();
                pop_back(token_stack, token_count);
            }
            else if (back(token_stack, token_count).IsWorkspaceJ()) {
                back(token_stack, token_count).workspacej.GetReader()->Enter();
                pop_back(token_stack, token_count);
            }
            x.now++;
            continue;

        OP_FUNC_QUIT:
            if (back(token_stack, token_count).IsWorkspace()) {
                back(token_stack, token_count).workspace.GetReader()->Quit();
                pop_back(token_stack, token_count);
            }
            else if (back(token_stack, token_count).IsWorkspaceJ()) {
                back(token_stack, token_count).workspacej.GetReader()->Quit();
                pop_back(token_stack, token_count);
            }
            x.now++;
            continue;

        OP_FUNC_SET_NAME:
            {
                auto name = back(token_stack, token_count).ToString();
                pop_back(token_stack, token_count);

                if (back(token_stack, token_count).IsWorkspace()) {
                    back(token_stack, token_count).workspace.GetReader()->SetKey(name);
                    pop_back(token_stack, token_count);
                }
                else if (back(token_stack, token_count).IsWorkspaceJ()) {
                    back(token_stack, token_count).workspacej.GetReader()->ChangeKey(claujson::_Value(name));
                    pop_back(token_stack, token_count);
                }
            }
            x.now++;
            continue;

        OP_FUNC_SET_VALUE:
            {
                auto value = back(token_stack, token_count).ToString();
                pop_back(token_stack, token_count);

                if (back(token_stack, token_count).IsWorkspace()) {
                    back(token_stack, token_count).workspace.GetReader()->SetData(value);
                    pop_back(token_stack, token_count);
                }
                else if (back(token_stack, token_count).IsWorkspaceJ()) {
                    claujson::Document d;
                    p.parse_str(value, d, 1);
                    back(token_stack, token_count).workspacej.GetReader()->Get() = std::move(d.Get());
                    pop_back(token_stack, token_count);
                }
            }
            x.now++;
            continue;

        OP_FUNC_GET_NAME:
            if (back(token_stack, token_count).IsWorkspace()) {
                Token token;
                token.SetCString(back(token_stack, token_count).workspace.GetReader()->GetKey().c_str());
                pop_back(token_stack, token_count);
                push_back(token_stack, token_count, std::move(token));
            }
            else if (back(token_stack, token_count).IsWorkspaceJ()) {
                Token token;
                token.SetCString(back(token_stack, token_count).workspacej.GetReader()->GetKey().get_string().data());
                pop_back(token_stack, token_count);
                push_back(token_stack, token_count, std::move(token));
            }
            x.now++;
            continue;

        OP_FUNC_GET_VALUE:
            if (back(token_stack, token_count).IsWorkspace()) {
                Token token;
                token.SetCString(back(token_stack, token_count).workspace.GetReader()->GetData().c_str());
                pop_back(token_stack, token_count);
                push_back(token_stack, token_count, std::move(token));
            }
            else if (back(token_stack, token_count).IsWorkspaceJ()) {
                Token token;
                auto& val = back(token_stack, token_count).workspacej.GetReader()->Get();
                if (val.is_int()) {
                    token.SetInt(val.get_integer());
                }
                else if (val.is_uint()) {
                    token.SetInt(val.get_unsigned_integer());
                }
                else if (val.is_float()) {
                    token.SetFloat(val.get_floating());
                }
                else if (val.is_bool()) {
                    token.SetBool(val.get_boolean());
                }
                else if (val.is_null()) {
                    token.SetCString("null");
                }
                else if (val.is_str()) {
                    token.SetCString(val.get_string().data());
                }
                else {
                    token.SetCString("");
                }

                pop_back(token_stack, token_count);
                push_back(token_stack, token_count, std::move(token));
            }
            x.now++;
            continue;

        OP_FUNC_GET_IDX:
            if (back(token_stack, token_count).IsWorkspace()) {
                Token token;
                token.SetInt(back(token_stack, token_count).workspace.GetReader()->GetIdx());
                pop_back(token_stack, token_count);
                push_back(token_stack, token_count, std::move(token));
            }
            else if (back(token_stack, token_count).IsWorkspaceJ()) {
                Token token;
                token.SetInt(back(token_stack, token_count).workspacej.GetReader()->GetIdx());
                pop_back(token_stack, token_count);
                push_back(token_stack, token_count, std::move(token));
            }
            x.now++;
            continue;

        OP_FUNC_SET_IDX:
            {
                auto a = back(token_stack, token_count).ToInt();
                pop_back(token_stack, token_count);

                if (back(token_stack, token_count).IsWorkspace()) {
                    auto& space = back(token_stack, token_count).workspace;
                    space.GetReader()->SetIndex(a);
                    pop_back(token_stack, token_count);
                }
                else if (back(token_stack, token_count).IsWorkspaceJ()) {
                    auto& space = back(token_stack, token_count).workspacej;
                    space.GetReader()->SetIdx(a);
                    pop_back(token_stack, token_count);
                }
            }
            x.now++;
            continue;

        OP_FUNC_WHILE:
            x.now++;
            continue;

        OP_FUNC_AND_ALL:
            {
                x.now++;
                auto param_count = x.code->event_data[x.now];
                bool result = true;

                for (int i = 0; i < param_count; i += 1) {
                    bool b = back(token_stack, token_count).ToBool();

                    if (result) {
                        result = result && b;
                    }

                    pop_back(token_stack, token_count);
                }

                Token temp;
                temp.SetBool(result);

                push_back(token_stack, token_count, std::move(temp));
            }
            x.now++;
            continue;

        OP_FUNC_AND:
            {
                bool result = true;

                for (int i = 0; i < 2; i += 1) {
                    bool b = back(token_stack, token_count).ToBool();
                    if (result) {
                        result = result && b;
                    }
                    pop_back(token_stack, token_count);
                }

                Token temp;
                temp.SetBool(result);

                push_back(token_stack, token_count, std::move(temp));
            }
            x.now++;
            continue;

        OP_FUNC_OR:
            {
                bool result = true;

                for (int i = 0; i < 2; i += 1) {
                    bool b = back(token_stack, token_count).ToBool();
                    result = result || b;
                    pop_back(token_stack, token_count);
                }

                Token temp;
                temp.SetBool(result);

                push_back(token_stack, token_count, std::move(temp));
            }
            x.now++;
            continue;

        OP_FUNC_GET_NOW:
            if (back(token_stack, token_count).IsWorkspace()) {
                auto& space = back(token_stack, token_count).workspace;

                Token temp;
                temp.SetWorkspace(space.GetReader());

                pop_back(token_stack, token_count);

                push_back(token_stack, token_count, std::move(temp));
            }
            else if (back(token_stack, token_count).IsWorkspaceJ()) {
                auto& space = back(token_stack, token_count).workspacej;

                Token temp;
                temp.SetWorkspaceJ(space.GetReader());

                pop_back(token_stack, token_count);

                push_back(token_stack, token_count, std::move(temp));
            }
            x.now++;
            continue;

        OP_THEN:
            {
                auto param = back(token_stack, token_count).ToBool();
                pop_back(token_stack, token_count);

                if (param) {
                    x.now++;
                }
                else {
                    x.now++;
                    x.now = x.code->event_data[x.now];
                    x.now--;
                }
            }
            x.now++;
            continue;

        OP_WHILE_END:
            {
                x.now++;
                x.now = x.code->event_data[x.now];
                x.now--;
            }
            x.now++;
            continue;

        OP_FUNC_IF:
            x.now++;
            continue;

        OP_IF_END:
            x.now++;
            continue;

        OP_FUNC_IS_END:
            {
                Token token;
                token.SetInt(x.return_value_now >= x.return_value.size());
                push_back(token_stack, token_count, std::move(token));
            }
            x.now++;
            continue;

        OP_FUNC_NOT:
            {
                auto a = std::move(back(token_stack, token_count));
                pop_back(token_stack, token_count);

                a.SetBool(!a.ToBool());

                push_back(token_stack, token_count, std::move(a));
            }
            x.now++;
            continue;

        OP_FUNC_NOT_EMPTY:
            {
                Token token = std::move(back(token_stack, token_count));
                pop_back(token_stack, token_count);

                if (token.IsWorkspace()) {
                    Token temp;
                    temp.SetBool(token.workspace.GetReader()->GetIdx() >= token.workspace.GetReader()->Length());
                    push_back(token_stack, token_count, std::move(temp));
                }
                else if (token.IsWorkspaceJ()) {
                    Token temp;
                    temp.SetBool(!token.workspacej.GetReader()->IsEndOfGroup());
                    push_back(token_stack, token_count, std::move(temp));
                }
            }
            x.now++;
            continue;

        OP_FUNC_IS_GROUP:
            if (back(token_stack, token_count).IsWorkspace()) {
                Token token;
                token.SetBool(back(token_stack, token_count).workspace.GetReader()->IsGroup());
                pop_back(token_stack, token_count);
                push_back(token_stack, token_count, std::move(token));
            }
            else if (back(token_stack, token_count).IsWorkspaceJ()) {
                Token token;
                token.SetBool(back(token_stack, token_count).workspacej.GetReader()->Get().is_structured());
                pop_back(token_stack, token_count);
                push_back(token_stack, token_count, std::move(token));
            }
            x.now++;
            continue;

        OP_FUNC_IS_ITEM:
            if (back(token_stack, token_count).IsWorkspace()) {
                Token token;
                token.SetBool(!back(token_stack, token_count).workspace.GetReader()->IsGroup());
                pop_back(token_stack, token_count);
                push_back(token_stack, token_count, std::move(token));
            }
            else if (back(token_stack, token_count).IsWorkspaceJ()) {
                Token token;
                token.SetBool(back(token_stack, token_count).workspacej.GetReader()->Get().is_primitive());
                pop_back(token_stack, token_count);
                push_back(token_stack, token_count, std::move(token));
            }
            x.now++;
            continue;

        OP_FUNC_GET_SIZE:
            if (back(token_stack, token_count).IsWorkspace()) {
                Token token;
                token.SetInt((back(token_stack, token_count).workspace.GetReader()->Length()));
                pop_back(token_stack, token_count);
                push_back(token_stack, token_count, std::move(token));
            }
            else if (back(token_stack, token_count).IsWorkspaceJ()) {
                Token token;
                token.SetInt((back(token_stack, token_count).workspacej.GetReader()->Size()));
                pop_back(token_stack, token_count);
                push_back(token_stack, token_count, std::move(token));
            }
            x.now++;
            continue;

        OP_FUNC_CLONE:
            if (back(token_stack, token_count).IsWorkspace()) {
                auto& a = back(token_stack, token_count).workspace;

                Token b;
                b.SetWorkspace(new clau_parser::Reader(*a.GetReader()));

                pop_back(token_stack, token_count);
                push_back(token_stack, token_count, std::move(b));
            }
            else if (back(token_stack, token_count).IsWorkspaceJ()) {
                auto& a = back(token_stack, token_count).workspacej;

                Token b;
                b.SetWorkspaceJ(new claujson::Explorer(*a.GetReader()));

                pop_back(token_stack, token_count);
                push_back(token_stack, token_count, std::move(b));
            }
            x.now++;
            continue;

        OP_FUNC_PRINT:
            {
                x.now++;

                std::vector<Token> vec;

                if (x.code->event_data[x.now] > 0) {
                    vec.reserve(x.code->event_data[x.now]);
                }

                for (int i = 0; i < x.code->event_data[x.now]; ++i) {
                    vec.push_back(std::move(back(token_stack, token_count)));
                    pop_back(token_stack, token_count);
                }
                for (int i = vec.size() - 1; i >= 0; --i) {
                    if (vec[i].ToString() == "\\n"sv) {
                        std::cout << "\n";
                    }
                    else {
                        std::cout << vec[i].ToString();
                    }
                }
            }
            x.now++;
            continue;
        }
    
        return return_value;
        */

	while (!empty(_stack_count)) {
		auto& x = back(_stack, _stack_count);
		count++;

		//std::cout << func_to_str[x.code->event_data[x.now]] << "\n";

		switch (x.code->event_data[x.now]) {
		case FUNC::TRUE:
		{
			Token token;
			token.SetBool(true);

			push_back(token_stack, token_count, std::move(token));
		}
		break;
		case FUNC::FALSE:
		{
			Token token;
			token.SetBool(false);

			push_back(token_stack, token_count, std::move(token));
		}
		break;
		case FUNC::FUNC_IS_INT:
		{
			auto token = std::move(back(token_stack, token_count)); pop_back(token_stack, token_count);

			token.SetBool(token.IsInt());

			push_back(token_stack, token_count, std::move(token));
		}
		break;
		case FUNC::FUNC_IS_FLOAT:
		{
			auto token = std::move(back(token_stack, token_count)); pop_back(token_stack, token_count);

			token.SetBool(token.IsFloat());

			push_back(token_stack, token_count, std::move(token));
		}
		break;
		case FUNC::FUNC_SPLIT:
		{
			auto divider = std::move(back(token_stack, token_count)); pop_back(token_stack, token_count);
			char ch = divider.ToString()[0];

			auto dir = std::move(back(token_stack, token_count)); pop_back(token_stack, token_count);
			auto& dest = global_var[dir.ToString()];

			auto str = back(token_stack, token_count).ToString(); pop_back(token_stack, token_count);

			auto arr = clau_parser::tokenize(str, ch);

			for (const auto& _ : arr) {
				Token token;
				token.SetString(_);
				dest.push_back(std::move(token));
			}
		}
		break;
		case FUNC::FUNC_GET_GLOBAL:
		{
			auto idx = back(token_stack, token_count).ToInt(); pop_back(token_stack, token_count);

			auto dir = back(token_stack, token_count).ToCString(); pop_back(token_stack, token_count);

			push_back(token_stack, token_count, global_var[dir][idx].clone());
		}
		break;
		case FUNC::FUNC_SET_GLOBAL:
		{
			auto value = std::move(back(token_stack, token_count)); pop_back(token_stack, token_count);
			auto idx = back(token_stack, token_count).ToInt(); pop_back(token_stack, token_count);
			auto dir = back(token_stack, token_count).ToString(); pop_back(token_stack, token_count);

			global_var[dir][idx] = std::move(value);
		}
		break;
		case FUNC::FUNC_MAKE_GLOBAL:
		{
			auto init = std::move(back(token_stack, token_count)); pop_back(token_stack, token_count);

			auto dir = std::move(back(token_stack, token_count)); pop_back(token_stack, token_count);
			
			global_var[dir.ToString()].push_back(std::move(init));
		}
		break;
		case FUNC::FUNC_CLEAR_GLOBAL:
		{
			auto dir = std::move(back(token_stack, token_count)); pop_back(token_stack, token_count);

			auto& dest = global_var[dir.ToString()];
	
			dest.clear();
		}
		break;	
		case FUNC::FUNC_REMOVE:
		{
			auto dir = std::move(back(token_stack, token_count)); pop_back(token_stack, token_count);

			auto arr = Find(global, dir.ToString());

			for (int i = 0; i < arr.size(); ++i) {
				auto ut = arr[i].workspace.GetReader()->GetUT();
				auto parent = ut->GetParent();

				for (int j = 0; j < parent->GetUserTypeListSize(); ++j) {
					if (parent->GetUserTypeList(j) == ut) {
						parent->RemoveUserTypeList(j);
					}
				}
			}
		}
		break; 
		case FUNC::FUNC_COUNT_GLOBAL:
		{
			auto dir = std::move(back(token_stack, token_count)); pop_back(token_stack, token_count);

			const auto& arr = global_var[dir.ToString()];
			Token token;
			token.SetInt(arr.size());
			push_back(token_stack, token_count, std::move(token));
		}
		break;
		case FUNC::FUNC_SEARCH:
		{
		//	x.now++;

            auto& ut = (x.code->input)[x.code->constant_data[x.now]]; //x.now++;
			auto to = std::move(back(token_stack, token_count)); pop_back(token_stack, token_count);
			auto dir = std::move(back(token_stack, token_count)); pop_back(token_stack, token_count);

			auto& workspace = this->Find(global, dir.ToString())[0].workspace;
			auto& dest = this->Find(global, to.ToString())[0].workspace;

			for (int i = 2; i < ut.ut_val->GetUserTypeListSize(); ++i) {
				this->SearchFunc(workspace.GetReader()->GetUT(), ut.ut_val->GetUserTypeList(i), dest.GetReader()->GetUT(), this);
			}
		}
		break;
		case FUNC::FUNC_QUERY:
		{
			//x.now++;

			auto& ut = (x.code->input)[x.code->constant_data[x.now]]; //x.now++;
			auto dir = std::move(back(token_stack, token_count)); pop_back(token_stack, token_count);

			auto& workspace = this->Find(global, dir.ToString())[0].workspace;


			for (int i = 1; i < ut.ut_val->GetUserTypeListSize(); ++i) {
				if (ut.ut_val->GetUserTypeList(i)->GetName() == "$insert"sv) {
					this->InsertFunc(workspace.GetReader()->GetUT(), ut.ut_val->GetUserTypeList(i), this);
				}
				else if (ut.ut_val->GetUserTypeList(i)->GetName() == "$update"sv) {
					this->UpdateFunc(workspace.GetReader()->GetUT(), ut.ut_val->GetUserTypeList(i), this);
				}
				else if (ut.ut_val->GetUserTypeList(i)->GetName() == "$delete"sv) {
					this->RemoveFunc(workspace.GetReader()->GetUT(), ut.ut_val->GetUserTypeList(i), this);
				}
			}
		}
		break;
		case FUNC::FUNC_GET:
		{
			auto token = std::move(back(token_stack, token_count));
			pop_back(token_stack, token_count);
			
			if (token.IsLocal()) { // ToString().starts_with("$local."sv)) {
				token.ConvertLocal();
				push_back(token_stack, token_count, x.local[token.ToCString()].clone());
			}
			else {
				auto value = FindValue(global, token.ToString()); // ToString?

				push_back(token_stack, token_count, std::move(value[0]));
			}
		}

		break;
		case FUNC::FUNC_ASSIGN:
		{
			auto value = std::move(back(token_stack, token_count));
			pop_back(token_stack, token_count);

			auto name = std::move(back(token_stack, token_count));
			pop_back(token_stack, token_count);

			if (name.IsLocal()) { // ToString().starts_with("$local."sv)) {
				x.local[name.ToCString()] = std::move(value);
			}
			else {
				auto idx = name.ToString().find_last_of('/');
				if (idx != std::string::npos) {
					auto dir = name.ToString().substr(0, idx);
					auto _name = name.ToString().substr(idx + 1);
					auto ut = Find(global, dir);

					for (int i = 0; i < ut.size(); ++i) {
						auto arr = ut[i].workspace.GetReader()->GetUT()->GetItemIdx(_name);
						
						for (int j = 0; j < arr.size(); ++j) {
							ut[i].workspace.GetReader()->GetUT()->GetItemList(arr[j]).Set(0, value.ToString());
						}
					}
				}
			}
		}
		break;
		case FUNC::START_DIR:
			count = 0;
			dir = "/";
			break;
		case FUNC::DIR:
			////std::cout << "DIR chk" << back(token_stack, token_count).ToString() << "\n";
		{
			auto& str = back(token_stack, token_count);

			if (str.IsParameter()) { //starts_with("$parameter."sv)) {
				str.ConvertParameter();
				Token& token = x.parameter[str.ToCString()];
				dir += token.ToString();
			}
			else if (str.IsLocal()) {
				str.ConvertLocal();
				Token& token = x.local[str.ToCString()];
				dir += token.ToString();
			}
			else {
				dir += back(token_stack, token_count).ToString(); // ToString
			}

			pop_back(token_stack, token_count);
			dir += "/";
		}

		break;
		case FUNC::END_DIR:
		{
			Token token;

			if (dir.back() == '/') {
				token.SetString(dir.substr(0, dir.size() - 1));
			}
			else {
				token.SetString(dir);
			}
			push_back(token_stack, token_count, std::move(token));

			dir = "";
			count = 0;
		}
		break;
		case FUNC::FUNC_REMOVE_QUOTED:
		{
			auto str = back(token_stack, token_count).ToString();

			if (str.size() >= 2) {
				str = str.substr(1, str.size() - 2);
			}
			pop_back(token_stack, token_count);

			Token temp;
			temp.SetString(str);
			push_back(token_stack, token_count, std::move(temp));
		}
		break;
		case FUNC::FUNC_IS_QUOTED_STR:
		{
			const auto& str = back(token_stack, token_count).ToString();
			bool chk = str.size() >= 2 && (str)[0] == str.back() && str.back() == '\"';

			pop_back(token_stack, token_count);

			Token temp;

			temp.SetBool(chk);

			push_back(token_stack, token_count, std::move(temp));
			break;
		}
		case FUNC::FUNC_RETURN:
			//x.now++;

			/*
			//std::cout << "return .... \n";)
			if (_stack.size() == 1) {
				std::vector<Token> temp;
				int count = x.code->event_data[x.now];

				for (int i = 0; i < count; ++i) {
					temp.push_back(std::move(back(token_stack, token_count)));
					pop_back(token_stack, token_count);
				}
				for (int i = 0; i < temp.size(); ++i) {
					return_value.push_back(std::move(temp[i]));
				}
			}*/
 			pop_back(_stack, _stack_count);
			break;
		case FUNC::CONSTANT:
			{
				const auto& value = (x.code->input)[x.code->constant_data[x.now]];
				//std::cout << value.ToString() << "\n";

				if (value.IsParameter()) { // ToString().starts_with("$parameter."sv)) {
					value.ConvertParameter();
					auto param = value.ToCString(); // .substr(11);

					push_back(token_stack, token_count, x.parameter.find(param)->clone());
				}
				else {
					push_back(token_stack, token_count, value.clone());
				}
			}

			break;
		case FUNC::FUNC_ADD:
		{
			auto a = std::move(back(token_stack, token_count));
			pop_back(token_stack, token_count);
			auto b = std::move(back(token_stack, token_count));
			pop_back(token_stack, token_count);

			{
				Token token;

				if (a.IsFloat() && b.IsFloat()) {
					token.SetFloat(a.ToFloat() + b.ToFloat());
				}
				else if (a.IsInt() && b.IsInt()) {
					token.SetInt(a.ToInt() + b.ToInt());
				}
				// todo...
				else if (a.IsFloat()) {
					
				}

				push_back(token_stack, token_count, std::move(token));
			}
		}
		break;
		case FUNC::FUNC_CALL:
			//x.now++;

			{
				auto count = x.code->constant_data[x.now];
                x.now++;

				Event e;

				for (int i = 0; i < count; ++i) {
					auto value = std::move(back(token_stack, token_count));
					pop_back(token_stack, token_count);
					auto name = std::move(back(token_stack, token_count));
					pop_back(token_stack, token_count);

					//std::cout << "name " << name.ToCString() << "\n";
					//std::cout << "value " << value.ToCString() << "\n";
					if (name.ToCString() == "id"sv) {
						//		//std::cout << e.id << "\n";
						if (value.IsInt()) {
							e.code = _event_code_list.find(value.ToInt());
                        }
						else {
							e.code = &_event_code_list[*_event_map.find(value.ToCString())];
                        }
						e.now = 0;
						continue;
					}

					e.parameter[name.ToCString()] = std::move(value); // name.ToString()
				}

				if (!e.code) {
					exit(-1);
				}
				push_back(_stack, _stack_count, std::move(e));
			}

			continue;

			break;

			// do not compare bools
		case FUNC::FUNC_COMP_LEFT:
			// Compare!
		{
			auto a = std::move(back(token_stack, token_count));
			pop_back(token_stack, token_count);
			auto b = std::move(back(token_stack, token_count));
			pop_back(token_stack, token_count);

			{
				Token token;

				if (a.IsFloat() && b.IsFloat()) {
					token.SetBool(b.ToFloat() > a.ToFloat());
				}
				else if (a.IsInt() && b.IsInt()) {
					token.SetBool(b.ToInt() > a.ToInt());
				}
				else if (a.IsString() && b.IsString()) {
					token.SetBool(b.ToString() > a.ToString());
				}

				push_back(token_stack, token_count, std::move(token));
			}
		}
		break;
		case FUNC::FUNC_COMP_RIGHT:
		{
			auto b = std::move(back(token_stack, token_count));
			pop_back(token_stack, token_count);
			auto a = std::move(back(token_stack, token_count));
			pop_back(token_stack, token_count);

			{
				Token token;

				if (a.IsFloat() && b.IsFloat()) {
					token.SetBool(a.ToFloat() < b.ToFloat());
				}
				else if (a.IsInt() && b.IsInt()) {
					token.SetBool(a.ToInt() < b.ToInt());
				}
				else if (a.IsString() && b.IsString()) {
					token.SetBool(a.ToString() < b.ToString());
				}

				push_back(token_stack, token_count, std::move(token));
			}
		}
		break;
		case FUNC::FUNC_FIND:
		{
            // chk error?
			auto a = std::move(back(token_stack, token_count));
			pop_back(token_stack, token_count);

            x.return_value = Find(global, a.ToString())[0].clone();
		}
		break;
		case FUNC::FUNC_RETURN_VALUE:
		{
			push_back(token_stack, token_count, x.return_value.clone());
		}
		break;
		case FUNC::FUNC_NEXT:
			//x.return_value_now++;
			break;
		case FUNC::FUNC_LOAD_DATA:
		{
			std::string fileName = back(token_stack, token_count).ToString();
			fileName = fileName.substr(1, fileName.size() - 2);
			pop_back(token_stack, token_count);

			clau_parser::UserType* dir = back(token_stack, token_count).workspace.GetReader()->GetUT();
			pop_back(token_stack, token_count);

			clau_parser::LoadData::LoadDataFromFile(fileName, *dir, 0, 0);
		}
		break;
		case FUNC::FUNC_LOAD_JSON:
		{
			std::string fileName = back(token_stack, token_count).ToString();
			fileName = fileName.substr(1, fileName.size() - 2);
			pop_back(token_stack, token_count);

			p.parse(fileName, d, 0);
			
			Token temp;
			temp.SetWorkspaceJ(new claujson::Explorer(d.Get()));
			x.return_value = (std::move(temp));
			//x.return_value_now = 0;
		}
		break;
		case FUNC::FUNC_ENTER:
			if (back(token_stack, token_count).IsWorkspace()) {
				back(token_stack, token_count).workspace.GetReader()->Enter();
				pop_back(token_stack, token_count);
			}
			else if (back(token_stack, token_count).IsWorkspaceJ()) {
				back(token_stack, token_count).workspacej.GetReader()->Enter();
				pop_back(token_stack, token_count);
			}
			break;
		case FUNC::FUNC_QUIT:
			if (back(token_stack, token_count).IsWorkspace()) {
				back(token_stack, token_count).workspace.GetReader()->Quit();
				pop_back(token_stack, token_count);
			}
			else if (back(token_stack, token_count).IsWorkspaceJ()) {
				back(token_stack, token_count).workspacej.GetReader()->Quit();
				pop_back(token_stack, token_count);
			}
			break;
		case FUNC::FUNC_SET_NAME:
		{
			auto name = back(token_stack, token_count).ToString();

			pop_back(token_stack, token_count);
			if (back(token_stack, token_count).IsWorkspace()) {
				back(token_stack, token_count).workspace.GetReader()->SetKey(name);

				pop_back(token_stack, token_count);
			}
			else if (back(token_stack, token_count).IsWorkspaceJ()) {
				back(token_stack, token_count).workspacej.GetReader()->ChangeKey(claujson::_Value(name));

				pop_back(token_stack, token_count);
			}
		}
		break;
		case FUNC::FUNC_SET_VALUE:
		{
			auto value = back(token_stack, token_count).ToString();

			pop_back(token_stack, token_count);
			if (back(token_stack, token_count).IsWorkspace()) {
				back(token_stack, token_count).workspace.GetReader()->SetData(value);

				pop_back(token_stack, token_count);
			}
			else if (back(token_stack, token_count).IsWorkspaceJ()) {
				claujson::Document d;
				p.parse_str(value, d, 1);
				back(token_stack, token_count).workspacej.GetReader()->Get() = std::move(d.Get());

				pop_back(token_stack, token_count);
			}
		}
		break;
		case FUNC::FUNC_GET_NAME:

			if (back(token_stack, token_count).IsWorkspace()) {
				Token token;
				token.SetCString(back(token_stack, token_count).workspace.GetReader()->GetKey().c_str());

				pop_back(token_stack, token_count);
				push_back(token_stack, token_count, std::move(token));
			} 
			else if (back(token_stack, token_count).IsWorkspaceJ()) {
				Token token;
				bool e = false; // error flag
				token.SetCString(back(token_stack, token_count).workspacej.GetReader()->GetKey().get_string().data());
				pop_back(token_stack, token_count);
				push_back(token_stack, token_count, std::move(token));
			}
		break;
		case FUNC::FUNC_GET_VALUE:

			if (back(token_stack, token_count).IsWorkspace()) {
				Token token;
				token.SetCString(back(token_stack, token_count).workspace.GetReader()->GetData().c_str());

				pop_back(token_stack, token_count);
				push_back(token_stack, token_count, std::move(token));
			}
			else if (back(token_stack, token_count).IsWorkspaceJ()) {
				Token token;
				auto& x = back(token_stack, token_count).workspacej.GetReader()->Get();
				if (x.is_int()) {
					token.SetInt(x.get_integer());
				}
				else if (x.is_uint()) {
					token.SetInt(x.get_unsigned_integer());
				}
				else if(x.is_float()) {
					token.SetFloat(x.get_floating());
				}
				else if (x.is_bool()) {
					token.SetBool(x.get_boolean());
				}
				else if (x.is_null()) {
					token.SetCString("null");
				}
				else if (x.is_str()) {
					token.SetCString(x.get_string().data());
				}
				else {
					// object or array
					// nothing to do
					token.SetCString("");
				}
				
				//token.SetString(back(token_stack, token_count).workspacej.GetReader()->Get().convert_primitive_to_std_string());

				pop_back(token_stack, token_count);
				push_back(token_stack, token_count, std::move(token));
			}
		break;
		case FUNC::FUNC_GET_IDX:
			if (back(token_stack, token_count).IsWorkspace()) {
				Token token;
				token.SetInt(back(token_stack, token_count).workspace.GetReader()->GetIdx());

				pop_back(token_stack, token_count);
				push_back(token_stack, token_count, std::move(token));
			}
			else if (back(token_stack, token_count).IsWorkspaceJ()) {
				Token token;
				token.SetInt(back(token_stack, token_count).workspacej.GetReader()->GetIdx());

				pop_back(token_stack, token_count);
				push_back(token_stack, token_count, std::move(token));
			}
		break;

		case FUNC::FUNC_SET_IDX:
		{
			//auto a = std::stoll(x.code->input[x.code->event_data[x.now]].ToString());

			auto a = back(token_stack, token_count).ToInt();
			pop_back(token_stack, token_count);
			if (back(token_stack, token_count).IsWorkspace()) {
				auto& space = back(token_stack, token_count).workspace;
				
				space.GetReader()->SetIndex(a);
				
				pop_back(token_stack, token_count);
			}
			else if (back(token_stack, token_count).IsWorkspaceJ()) {
				auto& space = back(token_stack, token_count).workspacej;
 			
				space.GetReader()->SetIdx(a);
				
				pop_back(token_stack, token_count);
			}
		}
		break;

		case FUNC::FUNC_WHILE:
		{
			//std::cout << "WHILE.... \n";
		}
		break;

		case FUNC::FUNC_AND_ALL:
		{
			//x.now++;
			auto count = x.code->constant_data[x.now]; //x.now++;
			bool result = true;

			for (int i = 0; i < count; i += 1) {
				bool b = back(token_stack, token_count).ToBool();
				
				if (result) {
					result = result && b;
				}

				pop_back(token_stack, token_count);
			}

			Token temp;
			temp.SetBool(result);

			push_back(token_stack, token_count, std::move(temp));
		}
		break;
		case FUNC::FUNC_AND:
		{
			bool result = true;

			for (int i = 0; i < 2; i += 1) {
				bool b = back(token_stack, token_count).ToBool();
				if (result) {
					result = result && b;
				}
				pop_back(token_stack, token_count);
			}

			Token temp;
			temp.SetBool(result);

			push_back(token_stack, token_count, std::move(temp));
		}

		break;
		case FUNC::FUNC_OR:
		{
			bool result = true;

			for (int i = 0; i < 2; i += 1) {
				bool b = back(token_stack, token_count).ToBool();
				result = result || b;
				pop_back(token_stack, token_count);
			}

			Token temp;

			temp.SetBool(result);

			push_back(token_stack, token_count, std::move(temp));
		}
		break;
		case FUNC::FUNC_GET_NOW:
			if (back(token_stack, token_count).IsWorkspace()) {
				auto& space = back(token_stack, token_count).workspace;
				
				Token temp;
				temp.SetWorkspace(space.GetReader());

				pop_back(token_stack, token_count);

				push_back(token_stack, token_count, std::move(temp));
			}
			else if (back(token_stack, token_count).IsWorkspaceJ()) {
				auto& space = back(token_stack, token_count).workspacej;
				

				Token temp;
				temp.SetWorkspaceJ(space.GetReader());

				pop_back(token_stack, token_count);

				push_back(token_stack, token_count, std::move(temp));
			}
		break;

		case FUNC::THEN:
		{
			auto param = back(token_stack, token_count).ToBool(); // bool
			pop_back(token_stack, token_count);

			if (param) {
				//x.now++;
			}
			else {
				x.now = x.code->constant_data[x.now]; 

				x.now--;
			}
		}
		break;
		case FUNC::WHILE_END:
		{
			x.now = x.code->constant_data[x.now];

			//std::cout << "chk .. " << func_to_str[x.code->event_data[x.now]] << "\n";

			x.now--;
		}
		break;
		case FUNC::FUNC_IF:
		{
			//
		}
		break;

		case FUNC::IF_END:
		{
			//
		}
		break;
		case FUNC::FUNC_IS_END: //
		{
			Token token;

			token.SetInt(1);

			push_back(token_stack, token_count, std::move(token));
		}
		break;
		case FUNC::FUNC_NOT:
		{
			auto a = std::move(back(token_stack, token_count));
			pop_back(token_stack, token_count);

			a.SetBool(!a.ToBool());

			push_back(token_stack, token_count, std::move(a));
		}
		break;
		case FUNC::FUNC_NOT_EMPTY:
		{
			Token token = std::move(back(token_stack, token_count));
			pop_back(token_stack, token_count);
			
			if (token.IsWorkspace()) {
				Token temp;
				temp.SetBool(token.workspace.GetReader()->GetIdx() >= token.workspace.GetReader()->Length());
				push_back(token_stack, token_count, std::move(temp));
			}
			else if (token.IsWorkspaceJ()) {
				Token temp;
				temp.SetBool(!token.workspacej.GetReader()->IsEndOfGroup());
				push_back(token_stack, token_count, std::move(temp));
			}
		}
		break;
		case FUNC::FUNC_IS_GROUP:
			if (back(token_stack, token_count).IsWorkspace()) {
				Token token;

				token.SetBool(back(token_stack, token_count).workspace.GetReader()->IsGroup());

				pop_back(token_stack, token_count);

				push_back(token_stack, token_count, std::move(token));
			}
			else if (back(token_stack, token_count).IsWorkspaceJ()) {
				Token token;

				token.SetBool(back(token_stack, token_count).workspacej.GetReader()->Get().is_structured()); // array or object

				pop_back(token_stack, token_count);

				push_back(token_stack, token_count, std::move(token));

			}
		break;
		case FUNC::FUNC_IS_ITEM:
			if (back(token_stack, token_count).IsWorkspace()) {
				Token token;

				token.SetBool(!back(token_stack, token_count).workspace.GetReader()->IsGroup());

				pop_back(token_stack, token_count);

				push_back(token_stack, token_count, std::move(token));
			}
			else if (back(token_stack, token_count).IsWorkspaceJ()) {
				Token token;

				token.SetBool(back(token_stack, token_count).workspacej.GetReader()->Get().is_primitive());

				pop_back(token_stack, token_count);

				push_back(token_stack, token_count, std::move(token));
			}
		break;
		case FUNC::FUNC_GET_SIZE: // for array or object?
			if (back(token_stack, token_count).IsWorkspace()) {
				Token token;
				token.SetInt((back(token_stack, token_count).workspace.GetReader()->Length()));
				pop_back(token_stack, token_count);

				push_back(token_stack, token_count, std::move(token));
			}
			else if (back(token_stack, token_count).IsWorkspaceJ()) {
				Token token;
				token.SetInt((back(token_stack, token_count).workspacej.GetReader()->Size()));
				pop_back(token_stack, token_count);

				push_back(token_stack, token_count, std::move(token));
			}
			break;
		case FUNC::FUNC_CLONE: // ?
			if (back(token_stack, token_count).IsWorkspace()) {
				auto& a = back(token_stack, token_count).workspace;
				
				Token b;
				b.SetWorkspace(new clau_parser::Reader(*a.GetReader()));
				
				pop_back(token_stack, token_count);
				push_back(token_stack, token_count, std::move(b));
			}
			else if (back(token_stack, token_count).IsWorkspaceJ()) {
				auto& a = back(token_stack, token_count).workspacej;
			
				Token b;
				b.SetWorkspaceJ(new claujson::Explorer(*a.GetReader()));
				
				pop_back(token_stack, token_count);
				push_back(token_stack, token_count, std::move(b));
			}
		break;

		case FUNC::FUNC_PRINT:
		{
			std::vector<Token> vec;

			if (x.code->constant_data[x.now] > 0) {
				vec.reserve(x.code->constant_data[x.now]);
			}

			for (int i = 0; i < x.code->constant_data[x.now]; ++i) {
				vec.push_back(std::move(back(token_stack, token_count)));
				pop_back(token_stack, token_count);
			}
			for (int i = vec.size() - 1; i >= 0; --i) {
				if (vec[i].ToString() == "\\n"sv) {
					std::cout << "\n";
				}
				else {
					std::cout << vec[i].ToString();
				}
			}
		}
		break;

        case FUNC::NOTHING:
        {
            break;
        }
		default:
			//std::cout << "error \n";
			break;

		}
		x.now++;
	}

	return return_value;
}
