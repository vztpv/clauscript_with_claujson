
#include "vm.h"


std::vector<Token> VM::Run(const std::string& id, clau_parser::UserType* global,
	const myMap<std::string, Token>& parameter) {
	claujson::parser p;
	claujson::Document d;

	Event main = _event_list[id];
	main.parameter = parameter;
	std::vector<Token> token_stack;
	std::vector<Event> _stack;
	
	myMap<std::string, std::vector<Token>> global_var;

	_stack.reserve(1024);
	_stack.push_back(main);
	int count = 0;
	std::string dir = "";

	std::vector<Token> return_value;

	while (!_stack.empty()) {
		auto& x = _stack.back();
		count++;

		//std::cout << func_to_str[x.event_data[x.now]] << "\n";

		switch (x.event_data[x.now]) {
		case FUNC::TRUE:
		{
			Token token;
			token.SetBool(true);

			token_stack.push_back(std::move(token));
		}
		break;
		case FUNC::FALSE:
		{
			Token token;
			token.SetBool(false);

			token_stack.push_back(std::move(token));
		}
		break;
		case FUNC::FUNC_IS_INT:
		{
			auto token = token_stack.back(); token_stack.pop_back();

			token.SetBool(token.IsInt());

			token_stack.push_back(std::move(token));
		}
		break;
		case FUNC::FUNC_IS_FLOAT:
		{
			auto token = token_stack.back(); token_stack.pop_back();

			token.SetBool(token.IsFloat());

			token_stack.push_back(std::move(token));
		}
		break;
		case FUNC::FUNC_SPLIT:
		{
			auto divider = token_stack.back(); token_stack.pop_back();
			char ch = divider.ToString()[0];

			auto dir = token_stack.back(); token_stack.pop_back();
			auto& dest = global_var[dir.ToString()];

			auto str = token_stack.back().ToString(); token_stack.pop_back();

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
			auto idx = token_stack.back().ToInt(); token_stack.pop_back();

			auto dir = token_stack.back().ToString(); token_stack.pop_back();

			token_stack.push_back(global_var[dir][idx]);
		}
		break;
		case FUNC::FUNC_SET_GLOBAL:
		{
			auto value = token_stack.back(); token_stack.pop_back();
			auto idx = token_stack.back().ToInt(); token_stack.pop_back();
			auto dir = token_stack.back().ToString(); token_stack.pop_back();

			global_var[dir][idx] = value;
		}
		break;
		case FUNC::FUNC_MAKE_GLOBAL:
		{
			auto init = token_stack.back(); token_stack.pop_back();

			auto dir = token_stack.back(); token_stack.pop_back();
			
			global_var[dir.ToString()].push_back(init);
		}
		break;
		case FUNC::FUNC_CLEAR_GLOBAL:
		{
			auto dir = token_stack.back(); token_stack.pop_back();

			auto& dest = global_var[dir.ToString()];
	
			dest.clear();
		}
		break;	
		case FUNC::FUNC_REMOVE:
		{
			auto dir = token_stack.back(); token_stack.pop_back();

			auto arr = Find(global, dir.ToString());

			for (int i = 0; i < arr.size(); ++i) {
				auto ut = arr[i].workspace.reader->GetUT();
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
			auto dir = token_stack.back(); token_stack.pop_back();

			const auto& arr = global_var[dir.ToString()];
			Token token;
			token.SetInt(arr.size());
			token_stack.push_back(std::move(token));
		}
		break;
		case FUNC::FUNC_SEARCH:
		{
			x.now++;

			auto ut = (*x.input)[x.event_data[x.now]];
			auto to = token_stack.back(); token_stack.pop_back();
			auto dir = token_stack.back(); token_stack.pop_back();

			auto workspace = this->Find(global, dir.ToString())[0].workspace;
			auto dest = this->Find(global, to.ToString())[0].workspace;

			for (int i = 2; i < ut.ut_val->GetUserTypeListSize(); ++i) {
				this->SearchFunc(workspace.reader->GetUT(), ut.ut_val->GetUserTypeList(i), dest.reader->GetUT(), this);
			}
		}
		break;
		case FUNC::FUNC_QUERY:
		{
			x.now++;

			auto ut = (*x.input)[x.event_data[x.now]];
			auto dir = token_stack.back(); token_stack.pop_back();

			auto workspace = this->Find(global, dir.ToString())[0].workspace;


			for (int i = 1; i < ut.ut_val->GetUserTypeListSize(); ++i) {
				if (ut.ut_val->GetUserTypeList(i)->GetName() == "$insert"sv) {
					this->InsertFunc(workspace.reader->GetUT(), ut.ut_val->GetUserTypeList(i), this);
				}
				else if (ut.ut_val->GetUserTypeList(i)->GetName() == "$update"sv) {
					this->UpdateFunc(workspace.reader->GetUT(), ut.ut_val->GetUserTypeList(i), this);
				}
				else if (ut.ut_val->GetUserTypeList(i)->GetName() == "$delete"sv) {
					this->RemoveFunc(workspace.reader->GetUT(), ut.ut_val->GetUserTypeList(i), this);
				}
			}
		}
		break;
		case FUNC::FUNC_GET:
		{
			auto token = token_stack.back();
			token_stack.pop_back();

			if (token.ToString().starts_with("$local."sv)) {

				token_stack.push_back(x.local[token.ToString().substr(7)]);
			}

			else {
				auto value = FindValue(global, token.ToString()); // ToString?

				token_stack.push_back(value[0]);
			}
		}

		break;
		case FUNC::FUNC_ASSIGN:
		{
			auto value = token_stack.back();
			token_stack.pop_back();

			auto name = token_stack.back();
			token_stack.pop_back();

			if (name.ToString().starts_with("$local."sv)) {
				x.local[name.ToString().substr(7)] = value;
			}
			else {
				auto idx = name.ToString().find_last_of('/');
				if (idx != std::string::npos) {
					auto dir = name.ToString().substr(0, idx);
					auto _name = name.ToString().substr(idx + 1);
					auto ut = Find(global, dir);

					for (int i = 0; i < ut.size(); ++i) {
						auto arr = ut[i].workspace.reader->GetUT()->GetItemIdx(_name);
						
						for (int j = 0; j < arr.size(); ++j) {
							ut[i].workspace.reader->GetUT()->GetItemList(arr[j]).Set(0, value.ToString());
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
			////std::cout << "DIR chk" << token_stack.back().ToString() << "\n";
		{
			auto str = token_stack.back().ToString();

			if (str.starts_with("$parameter."sv)) {
				str = str.substr(11);

				Token token = x.parameter[str];
				dir += token.ToString();
			}
			else if (str.starts_with("$local."sv)) {
				str = str.substr(7);

				Token token = x.local[str];
				dir += token.ToString();
			}
			else {
				dir += token_stack.back().ToString(); // ToString
			}

			token_stack.pop_back();
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
			token_stack.push_back(std::move(token));

			dir = "";
			count = 0;
		}
		break;
		case FUNC::FUNC_REMOVE_QUOTED:
		{
			auto str = token_stack.back().ToString();

			if (str.size() >= 2) {
				str = str.substr(1, str.size() - 2);
			}
			token_stack.pop_back();

			Token temp;
			temp.SetString(str);
			token_stack.push_back(std::move(temp));
		}
		break;
		case FUNC::FUNC_IS_QUOTED_STR:
		{
			const auto& str = token_stack.back().ToString();
			bool chk = str.size() >= 2 && (str)[0] == str.back() && str.back() == '\"';

			token_stack.pop_back();

			Token temp;

			temp.SetBool(chk);

			token_stack.push_back(std::move(temp));
			break;
		}
		case FUNC::FUNC_RETURN:
			x.now++;

			//std::cout << "return .... \n";)
			if (_stack.size() == 1) {
				std::vector<Token> temp;
				int count = x.event_data[x.now];

				for (int i = 0; i < count; ++i) {
					temp.push_back(token_stack.back());
					token_stack.pop_back();
				}
				for (int i = 0; i < temp.size(); ++i) {
					return_value.push_back(temp[i]);
				}
			}
 			_stack.pop_back();
			break;
		case FUNC::CONSTANT:
			x.now++;

			{
				const auto& value = (*x.input)[x.event_data[x.now]];
				//std::cout << value.ToString() << "\n";

				if (value.IsString()) {
					if (value.ToString().starts_with("$parameter."sv)) {
						auto param = value.ToString().substr(11);

						token_stack.push_back(x.parameter[param]);

						x.now++;
						continue;
					}
				}

				{
					token_stack.push_back(value);
				}
			}

			break;
		case FUNC::FUNC_ADD:
		{
			auto a = token_stack.back();
			token_stack.pop_back();
			auto b = token_stack.back();
			token_stack.pop_back();

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

				token_stack.push_back(std::move(token));
			}
		}
		break;
		case FUNC::FUNC_CALL:
			x.now++;

			{
				auto count = x.event_data[x.now];

				x.now++;

				Event e;

				for (int i = 0; i < count; ++i) {
					auto value = token_stack.back();
					token_stack.pop_back();
					auto name = token_stack.back();
					token_stack.pop_back();

					if (name.ToString() == "id"sv) {
						e.id = value.ToString();

						//		//std::cout << e.id << "\n";

						e.event_data = _event_list[value.ToString()].event_data;
						e.input = _event_list[value.ToString()].input;
						e.now = 0;
						e.return_value_now = 0;
						continue;
					}

					e.parameter[name.ToString()] = (value); // name.ToString()
				}

				////std::cout << "call " << e.id << "\n";
				_stack.push_back(std::move(e));
			}

			continue;

			break;

			// do not compare bools
		case FUNC::FUNC_COMP_LEFT:
			// Compare!
		{
			auto a = token_stack.back();
			token_stack.pop_back();
			auto b = token_stack.back();
			token_stack.pop_back();

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

				token_stack.push_back(std::move(token));
			}
		}
		break;
		case FUNC::FUNC_COMP_RIGHT:
		{
			auto b = token_stack.back();
			token_stack.pop_back();
			auto a = token_stack.back();
			token_stack.pop_back();

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

				token_stack.push_back(std::move(token));
			}
		}
		break;
		case FUNC::FUNC_FIND:
		{
			auto a = token_stack.back();
			token_stack.pop_back();

			x.return_value = Find(global, a.ToString());
			x.return_value_now = 0;
		}
		break;
		case FUNC::FUNC_RETURN_VALUE:
		{
			token_stack.push_back(x.return_value[x.return_value_now]);
		}
		break;
		case FUNC::FUNC_NEXT:
			x.return_value_now++;
			break;
		case FUNC::FUNC_LOAD_DATA:
		{
			std::string fileName = token_stack.back().ToString();
			fileName = fileName.substr(1, fileName.size() - 2);
			token_stack.pop_back();

			clau_parser::UserType* dir = token_stack.back().workspace.reader->GetUT();
			token_stack.pop_back();

			clau_parser::LoadData::LoadDataFromFile(fileName, *dir, 0, 0);
		}
		break;
		case FUNC::FUNC_LOAD_JSON:
		{
			std::string fileName = token_stack.back().ToString();
			fileName = fileName.substr(1, fileName.size() - 2);
			token_stack.pop_back();

			p.parse(fileName, d, 0);
			
			x.return_value.clear();
			token_stack.back().workspacej.reader = 
				wiz::SmartPtr<claujson::Explorer>(new claujson::Explorer(d.Get()));

			Token temp;
			temp.SetWorkspaceJ(token_stack.back().workspacej.reader);
			x.return_value.push_back(std::move(temp));
			x.return_value_now = 0;
		}
		break;
		case FUNC::FUNC_ENTER:
			if (token_stack.back().IsWorkspace()) {
				token_stack.back().workspace.reader->Enter();
				token_stack.pop_back();
			}
			else if (token_stack.back().IsWorkspaceJ()) {
				token_stack.back().workspacej.reader->Enter();
				token_stack.pop_back();
			}
			break;
		case FUNC::FUNC_QUIT:
			if (token_stack.back().IsWorkspace()) {
				token_stack.back().workspace.reader->Quit();
				token_stack.pop_back();
			}
			else if (token_stack.back().IsWorkspaceJ()) {
				token_stack.back().workspacej.reader->Quit();
				token_stack.pop_back();
			}
			break;
		case FUNC::FUNC_SET_NAME:
		{
			auto name = token_stack.back().ToString();

			token_stack.pop_back();
			if (token_stack.back().IsWorkspace()) {
				token_stack.back().workspace.reader->SetKey(name);

				token_stack.pop_back();
			}
			else if (token_stack.back().IsWorkspaceJ()) {
				token_stack.back().workspacej.reader->ChangeKey(claujson::_Value(name));

				token_stack.pop_back();
			}
		}
		break;
		case FUNC::FUNC_SET_VALUE:
		{
			auto value = token_stack.back().ToString();

			token_stack.pop_back();
			if (token_stack.back().IsWorkspace()) {
				token_stack.back().workspace.reader->SetData(value);

				token_stack.pop_back();
			}
			else if (token_stack.back().IsWorkspaceJ()) {
				claujson::Document d;
				p.parse_str(value, d, 1);
				token_stack.back().workspacej.reader->Get() = std::move(d.Get());

				token_stack.pop_back();
			}
		}
		break;
		case FUNC::FUNC_GET_NAME:

			if (token_stack.back().IsWorkspace()) {
				Token token;
				token.SetString(token_stack.back().workspace.reader->GetKey());

				token_stack.pop_back();
				token_stack.push_back(std::move(token));
			} 
			else if (token_stack.back().IsWorkspaceJ()) {
				Token token;
				bool e = false; // error flag
				token.SetString(token_stack.back().workspacej.reader->GetKey().get_string().get_std_string(e));
				token.SetString("\"" + token.ToString() + "\"");
				token_stack.pop_back();
				token_stack.push_back(std::move(token));
			}
		break;
		case FUNC::FUNC_GET_VALUE:

			if (token_stack.back().IsWorkspace()) {
				Token token;
				token.SetString(token_stack.back().workspace.reader->GetData());

				token_stack.pop_back();
				token_stack.push_back(std::move(token));
			}
			else if (token_stack.back().IsWorkspaceJ()) {
				Token token;
				token.SetString(token_stack.back().workspacej.reader->Get().convert_primitive_to_std_string());
				if (token_stack.back().workspacej.reader->Get().is_str()) {
					token.SetString("\"" + token.ToString() + "\"");
				}
				token_stack.pop_back();
				token_stack.push_back(std::move(token));
			}
		break;
		case FUNC::FUNC_GET_IDX:
			if (token_stack.back().IsWorkspace()) {
				Token token;
				token.SetInt(token_stack.back().workspace.reader->GetIdx());

				token_stack.pop_back();
				token_stack.push_back(std::move(token));
			}
			else if (token_stack.back().IsWorkspaceJ()) {
				Token token;
				token.SetInt(token_stack.back().workspacej.reader->GetIdx());

				token_stack.pop_back();
				token_stack.push_back(std::move(token));
			}
		break;

		case FUNC::FUNC_SET_IDX:
		{
			//auto a = std::stoll(x.input[x.event_data[x.now]].ToString());

			auto a = token_stack.back().ToInt();
			token_stack.pop_back();
			if (token_stack.back().IsWorkspace()) {
				auto space = token_stack.back().workspace;
				token_stack.pop_back();

				space.reader->SetIndex(a);
			}
			else if (token_stack.back().IsWorkspaceJ()) {
				auto space = token_stack.back().workspacej;
 				token_stack.pop_back();

				space.reader->SetIdx(a);
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
			x.now++;
			auto count = x.event_data[x.now];
			bool result = true;

			for (int i = 0; i < count; i += 1) {
				bool b = token_stack.back().ToBool();
				
				if (result) {
					result = result && b;
				}

				token_stack.pop_back();
			}

			Token temp;
			temp.SetBool(result);

			token_stack.push_back(temp);
		}
		break;
		case FUNC::FUNC_AND:
		{
			bool result = true;

			for (int i = 0; i < 2; i += 1) {
				bool b = token_stack.back().ToBool();
				if (result) {
					result = result && b;
				}
				token_stack.pop_back();
			}

			Token temp;
			temp.SetBool(result);

			token_stack.push_back(temp);
		}

		break;
		case FUNC::FUNC_OR:
		{
			bool result = true;

			for (int i = 0; i < 2; i += 1) {
				bool b = token_stack.back().ToBool();
				result = result || b;
				token_stack.pop_back();
			}

			Token temp;

			temp.SetBool(result);

			token_stack.push_back(temp);
		}
		break;
		case FUNC::FUNC_GET_NOW:
			if (token_stack.back().IsWorkspace()) {
				auto space = token_stack.back().workspace;
				token_stack.pop_back();

				Token temp;
				temp.SetWorkspace(space.reader);

				token_stack.push_back(temp);
			}
			else if (token_stack.back().IsWorkspaceJ()) {
				auto space = token_stack.back().workspacej;
				token_stack.pop_back();

				Token temp;
				temp.SetWorkspaceJ(space.reader);

				token_stack.push_back(temp);
			}
		break;

		case FUNC::THEN:
		{
			auto param = token_stack.back().ToBool(); // bool
			token_stack.pop_back();

			if (param) {
				x.now++;
			}
			else {
				x.now++;

				x.now = x.event_data[x.now];

				x.now--;
			}
		}
		break;
		case FUNC::WHILE_END:
		{
			x.now++;
			x.now = x.event_data[x.now];

			//std::cout << "chk .. " << func_to_str[x.event_data[x.now]] << "\n";

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
		case FUNC::FUNC_IS_END:
		{
			Token token;

			token.SetInt(x.return_value_now >= x.return_value.size());

			token_stack.push_back(std::move(token));
		}
		break;
		case FUNC::FUNC_NOT:
		{
			auto a = token_stack.back();
			token_stack.pop_back();

			a.SetBool(!a.ToBool());

			token_stack.push_back(a);
		}
		break;
		case FUNC::FUNC_NOT_EMPTY:
		{
			Token token = token_stack.back();
			token_stack.pop_back();
			
			if (token.IsWorkspace()) {
				Token temp;
				temp.SetBool(token.workspace.reader->GetIdx() >= token.workspace.reader->Length());
				token_stack.push_back(temp);
			}
			else if (token.IsWorkspaceJ()) {
				Token temp;
				temp.SetBool(!token.workspacej.reader->IsEndOfGroup());
				token_stack.push_back(temp);
			}
		}
		break;
		case FUNC::FUNC_IS_GROUP:
			if (token_stack.back().IsWorkspace()) {
				Token token;

				token.SetBool(token_stack.back().workspace.reader->IsGroup());

				token_stack.pop_back();

				token_stack.push_back(std::move(token));
			}
			else if (token_stack.back().IsWorkspaceJ()) {
				Token token;

				token.SetBool(token_stack.back().workspacej.reader->Get().is_structured()); // array or object

				token_stack.pop_back();

				token_stack.push_back(std::move(token));

			}
		break;
		case FUNC::FUNC_IS_ITEM:
			if (token_stack.back().IsWorkspace()) {
				Token token;

				token.SetBool(!token_stack.back().workspace.reader->IsGroup());

				token_stack.pop_back();

				token_stack.push_back(std::move(token));
			}
			else if (token_stack.back().IsWorkspaceJ()) {
				Token token;

				token.SetBool(token_stack.back().workspacej.reader->Get().is_primitive());

				token_stack.pop_back();

				token_stack.push_back(std::move(token));
			}
		break;
		case FUNC::FUNC_GET_SIZE: // for array or object?
			if (token_stack.back().IsWorkspace()) {
				Token token;
				token.SetString(std::to_string(token_stack.back().workspace.reader->Length()));
				token_stack.pop_back();

				token_stack.push_back(std::move(token));
			}
			else if (token_stack.back().IsWorkspaceJ()) {
				Token token;
				token.SetString(std::to_string(token_stack.back().workspacej.reader->Size()));
				token_stack.pop_back();

				token_stack.push_back(std::move(token));
			}
			break;
		case FUNC::FUNC_CLONE: // ?
			if (token_stack.back().IsWorkspace()) {
				auto a = token_stack.back().workspace;
				token_stack.pop_back();

				Token b;
				b.SetWorkspace(new clau_parser::Reader(*a.reader));
				token_stack.push_back(b);
			}
			else if (token_stack.back().IsWorkspaceJ()) {
				auto a = token_stack.back().workspacej;
				token_stack.pop_back();

				Token b;
				b.SetWorkspaceJ(new claujson::Explorer(*a.reader));
				token_stack.push_back(b);
			}
		break;

		case FUNC::FUNC_PRINT:
		{
			x.now++;

			std::vector<Token> vec;

			if (x.event_data[x.now] > 0) {
				vec.reserve(x.event_data[x.now]);
			}

			for (int i = 0; i < x.event_data[x.now]; ++i) {
				vec.push_back(token_stack.back());
				token_stack.pop_back();
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
		default:
			//std::cout << "error \n";
			break;;

		}
		x.now++;
	}

	return return_value;
}
