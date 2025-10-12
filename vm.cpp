#pragma once

#include "vm.h"

std::vector<Token> FindValue(clau_parser::UserType* ut, const std::string& str)
{ // std::string 대신 vector<std::string> ??
	int count = 0;
	int idx = -1;
	for (int i = str.size() - 1; i >= 0; --i) {
		if ('/' == str[i]) {
			if (count == 0) {
				idx = i;
			}
			count++;
		}
	}

	std::vector<Token> result;

	if (count == 1)
	{
		Token token;
		token.SetString("");
		result.push_back(std::move(token));
		return result;
	}
	else {
		auto x = clau_parser::UserType::Find(ut, str.substr(0, idx + 1));

		if (x.first == false) { return result; }

		for (int i = 0; i < x.second.size(); ++i) {
			std::string itemName = str.substr(idx + 1);

			if (itemName.starts_with("%it") && itemName.size() >= 4) {
				int itemIdx = std::stoi(itemName.substr(3));

				Token temp;
				temp.SetString(x.second[i]->GetItemList(itemIdx).Get(0).c_str());
				result.push_back(std::move(temp));
			}
			else {
				if (itemName == "_") {
					itemName = "";
				}
				auto temp = x.second[i]->GetItem(itemName);
				if (!temp.empty()) {
					for (int j = 0; j < temp.size(); ++j) {
						Token tkn;
						tkn.SetString(temp[j].Get(0).c_str());

						result.push_back(std::move(tkn));
					}
				}
			}
		}
	}
	return result;
}

void VM::_MakeByteCode(clau_parser::UserType* ut, Event* e, EventCode* code, std::string* id) {
	long long it_count = 0, ut_count = 0;
	for (long long i = 0; i < ut->GetIListSize(); ++i) {
		if (ut->IsItemList(i)) {
			if (!ut->GetItemList(it_count).GetName().empty()) {
				if (ut->GetItemList(it_count).GetName() == "id"sv && ut->GetName() == "Event"sv) {
					if (id) {
						*id = ut->GetItemList(it_count).Get();
					}
					if (_event_map.find((ut->GetItemList(it_count).Get().c_str())) == nullptr) {
						_event_map.insert({(ut->GetItemList(it_count).Get().c_str()), _event_map.size() });
					}
					
					it_count++;
					continue;
				}

				{
					Token token(ut);

					token.SetString(ut->GetItemList(it_count).GetName().c_str());

					code->input.push_back(std::move(token));
				}

				code->event_data.push_back(FUNC::CONSTANT);
				code->constant_data.push_back(code->input.size() - 1);

				{
					auto a = ut->GetItemList(it_count).Get();

					if (a.starts_with("/")) {

						code->event_data.push_back(FUNC::START_DIR);
						code->constant_data.push_back(0);

						auto tokens = clau_parser::tokenize(a, '/');

						for (int i = 0; i < tokens.size(); ++i) {
							if (tokens[i].starts_with("$"sv) && !tokens[i].starts_with("$parameter."sv) && !tokens[i].starts_with("$local."sv)) {
								clau_parser::UserType new_ut;

								new_ut.AddUserTypeItem(clau_parser::UserType(tokens[i]));

								_MakeByteCode(&new_ut, e, code);
							}
							else {
								Token token(ut);
								token.SetString(tokens[i].c_str());

								code->input.push_back(std::move(token));
								code->event_data.push_back(FUNC::CONSTANT);
								code->constant_data.push_back(code->input.size() - 1);
								code->event_data.push_back(FUNC::DIR);
								code->constant_data.push_back(0);
							}
						}

						code->event_data.push_back(FUNC::END_DIR);
						code->constant_data.push_back(0);
					}
					else if (a.starts_with("@")) {
						auto tokens = clau_parser::tokenize(a, '@');

						for (int i = 0; i < tokens.size(); ++i) {
							if (tokens[i].starts_with("$"sv) && !tokens[i].starts_with("$parameter."sv) && !tokens[i].starts_with("$local."sv)) {
								clau_parser::UserType new_ut;

								new_ut.AddUserTypeItem(clau_parser::UserType(tokens[i]));

								_MakeByteCode(&new_ut, e, code);
								break;
							}
							else {
								Token token(ut);
								token.SetString(tokens[i].c_str());

								code->input.push_back(std::move(token));
								code->event_data.push_back(FUNC::CONSTANT);
								code->constant_data.push_back(code->input.size() - 1);
							}
						}
					}
					else {
						Token token;

						if (ut->GetItemList(it_count).GetName() == "id"sv && !ut->GetItemList(it_count).Get().starts_with("$")) {
							if (this->_event_map.find(ut->GetItemList(it_count).Get().c_str()) == nullptr) {
								this->_event_map.insert({ ut->GetItemList(it_count).Get().c_str(), this->_event_map.size()});
							}
							token.SetInt(this->_event_map[ut->GetItemList(it_count).Get().c_str()]);
						}
						else {
							token.SetString(ut->GetItemList(it_count).Get().c_str());
						}

						code->input.push_back(std::move(token));
						
						code->event_data.push_back(FUNC::CONSTANT);
						code->constant_data.push_back(code->input.size() - 1);
					}
				}
			}

			// $while, $if
			else if (ut->GetItemList(it_count).GetName().empty()) {
				if (ut->GetItemList(it_count).Get() == "$while"sv) {
					code->event_data.push_back(FUNC::FUNC_WHILE);
					code->constant_data.push_back(0);
					int idx = code->event_data.size() - 1;
					int then_idx = 0;

					_MakeByteCode(ut->GetUserTypeList(ut_count), e, code);

					ut_count++; ++i;

					code->event_data.push_back(FUNC::THEN);
					code->constant_data.push_back(0);
					then_idx = code->event_data.size() - 1;
					{
						//Event _e;
						//int count2 = _MakeByteCode(ut->GetUserTypeList(ut_count), &_e);
						//count2;
						//code->event_data.push_back(0); 
					}

					_MakeByteCode(ut->GetUserTypeList(ut_count), e, code);

					ut_count++; ++i;

					code->event_data.push_back(FUNC::WHILE_END);
					code->constant_data.push_back(idx);
					code->constant_data[then_idx] = code->event_data.size(); //
				}
				else if (ut->GetItemList(it_count).Get() == "$if"sv) {
					code->event_data.push_back(FUNC::FUNC_IF);
					code->constant_data.push_back(0);
					int idx = 0;

					_MakeByteCode(ut->GetUserTypeList(ut_count), e, code);
					ut_count++; ++i;

					code->event_data.push_back(FUNC::THEN);
					code->constant_data.push_back(0);
					idx = code->event_data.size() - 1;

					_MakeByteCode(ut->GetUserTypeList(ut_count), e, code);

					ut_count++; ++i;

					code->event_data.push_back(FUNC::IF_END);
					code->constant_data.push_back(0);
					code->constant_data[idx] = code->event_data.size(); //
				}
				else if (ut->GetItemList(it_count).Get() == "TRUE"sv) {
					code->event_data.push_back(FUNC::TRUE);
					code->constant_data.push_back(0);
				}
				else if (ut->GetItemList(it_count).Get() == "FALSE"sv) {
					code->event_data.push_back(FUNC::FALSE);
					code->constant_data.push_back(0);
				}
				else {
					auto a = ut->GetItemList(it_count).Get();

					if (a.starts_with("/")) {

						code->event_data.push_back(FUNC::START_DIR); 
						code->constant_data.push_back(0);

						auto tokens = clau_parser::tokenize(a, '/');

						for (int i = 0; i < tokens.size(); ++i) {
							if (tokens[i].starts_with("$"sv) && !tokens[i].starts_with("$parameter."sv) && !tokens[i].starts_with("$local."sv)) {
								clau_parser::UserType new_ut;

								new_ut.AddUserTypeItem(clau_parser::UserType(tokens[i]));

								_MakeByteCode(&new_ut, e, code);
								break;
							}
							else {
								Token token(ut);

								token.SetString(tokens[i].c_str());

								code->input.push_back(std::move(token));
								code->event_data.push_back(FUNC::CONSTANT);
								code->constant_data.push_back(code->input.size() - 1);
								code->event_data.push_back(FUNC::DIR);
								code->constant_data.push_back(0);
							}
						}

						code->event_data.push_back(FUNC::END_DIR);
						code->constant_data.push_back(0);
					}
					else if (a.starts_with("@")) {
						auto tokens = clau_parser::tokenize(a, '@');

						for (int i = 0; i < tokens.size(); ++i) {
							if (tokens[i].starts_with("$"sv) && !tokens[i].starts_with("$parameter."sv) && !tokens[i].starts_with("$local."sv)) {
								clau_parser::UserType new_ut;

								new_ut.AddUserTypeItem(clau_parser::UserType(tokens[i]));

								_MakeByteCode(&new_ut, e, code);
							}
							else {
								Token token;

								token.SetString(tokens[i].c_str());

								code->input.push_back(std::move(token));
								code->event_data.push_back(FUNC::CONSTANT);
								code->constant_data.push_back(code->input.size() - 1);
							}
						}
					}
					else {
						Token token;

						token.SetString(ut->GetItemList(it_count).Get().c_str());

						code->input.push_back(std::move(token));

						code->event_data.push_back(FUNC::CONSTANT);
						code->constant_data.push_back(code->input.size() - 1);
					}
				}
			}

			it_count++;
		}
		else {
			std::string& name = ut->GetUserTypeList(ut_count)->GetName();

			if (name != "$query"sv && name != "$search"sv) {
				_MakeByteCode(ut->GetUserTypeList(ut_count), e, code);
			}

			if (!ut->GetUserTypeList(ut_count)->GetName().empty()) {
				if (name.starts_with("$"sv)) {
					Token token;

					token.SetFunc(); // | Token::Type::UserType

					if (name == "$is_int"sv) {
						token.func = FUNC::FUNC_IS_INT;

						code->event_data.push_back(FUNC::FUNC_IS_INT);
						code->constant_data.push_back(0);
					}
					else if (name == "$is_float"sv) {
						token.func = FUNC::FUNC_IS_FLOAT;

						code->event_data.push_back(FUNC::FUNC_IS_FLOAT);
						code->constant_data.push_back(0);
					}
					else if (name == "$split"sv) {
						token.func = FUNC::FUNC_SPLIT;

						code->event_data.push_back(FUNC::FUNC_SPLIT);
						code->constant_data.push_back(0);
					}
					else if (name == "$make_global"sv) {
						token.func = FUNC::FUNC_MAKE_GLOBAL;

						code->event_data.push_back(FUNC::FUNC_MAKE_GLOBAL);
						code->constant_data.push_back(0);
					}
					else if (name == "$clear_global"sv) {
						token.func = FUNC::FUNC_CLEAR_GLOBAL;

						code->event_data.push_back(FUNC::FUNC_CLEAR_GLOBAL);
						code->constant_data.push_back(0);
					}
					else if (name == "$remove"sv) {
						token.func = FUNC::FUNC_REMOVE;

						code->event_data.push_back(FUNC::FUNC_REMOVE);
						code->constant_data.push_back(0);
					}
					else if (name == "$count_global"sv) {
						token.func = FUNC::FUNC_COUNT_GLOBAL;

						code->event_data.push_back(FUNC::FUNC_COUNT_GLOBAL);
						code->constant_data.push_back(0);
					}
					else if (name == "$search"sv) {
						_MakeByteCode(ut->GetUserTypeList(ut_count)->GetUserTypeList(0), e, code);
						_MakeByteCode(ut->GetUserTypeList(ut_count)->GetUserTypeList(1), e, code);

						token.func = FUNC::FUNC_SEARCH;

						code->event_data.push_back(FUNC::FUNC_SEARCH);
						
						{
							Token temp;
							temp.ut_val = wiz::SmartPtr<clau_parser::UserType>(new clau_parser::UserType(*ut->GetUserTypeList(ut_count)));

							code->input.push_back(std::move(temp));
							code->constant_data.push_back(code->input.size() - 1);
						}
					}
					else if (name == "$query"sv) {
						_MakeByteCode(ut->GetUserTypeList(ut_count)->GetUserTypeList(0), e, code);

						token.func = FUNC::FUNC_QUERY;

						code->event_data.push_back(FUNC::FUNC_QUERY);
						{
							Token temp;
							temp.ut_val = wiz::SmartPtr<clau_parser::UserType>(new clau_parser::UserType(*ut->GetUserTypeList(ut_count)));

							code->input.push_back(std::move(temp));
							code->constant_data.push_back(code->input.size() - 1);
						}
					}
					else if (name == "$clone"sv) {
						token.func = FUNC::FUNC_CLONE;

						code->event_data.push_back(FUNC::FUNC_CLONE);
						code->constant_data.push_back(0);
					}
					else if (name == "$call"sv) {
						token.func = FUNC::FUNC_CALL;

						code->event_data.push_back(FUNC::FUNC_CALL);
						code->constant_data.push_back(ut->GetUserTypeList(ut_count)->GetItemListSize());
					}
					else if (name == "$not_empty"sv) {
						token.func = FUNC::FUNC_NOT_EMPTY;
						code->event_data.push_back(FUNC::FUNC_NOT_EMPTY);
						code->constant_data.push_back(0);
					}
					else if (name == "$set_idx"sv) {
						token.func = FUNC::FUNC_SET_IDX;

						code->event_data.push_back(FUNC::FUNC_SET_IDX);
						code->constant_data.push_back(0);
					}
					else if (name == "$add"sv) {
						token.func = FUNC::FUNC_ADD;

						code->event_data.push_back(FUNC::FUNC_ADD);
						code->constant_data.push_back(0);
					}
					else if (name == "$get"sv) {
						token.func = FUNC::FUNC_GET;

						code->event_data.push_back(FUNC::FUNC_GET);
						code->constant_data.push_back(0);
					}
					else if (name == "$set_global"sv) {
						token.func = FUNC::FUNC_SET_GLOBAL;
						code->event_data.push_back(FUNC::FUNC_SET_GLOBAL);
						code->constant_data.push_back(0);
					}
					else if (name == "$get_global"sv) {
						token.func = FUNC::FUNC_GET_GLOBAL;

						code->event_data.push_back(FUNC::FUNC_GET_GLOBAL);
						code->constant_data.push_back(0);
					}
					else if (name == "$get_name"sv) {
						token.func = FUNC::FUNC_GET_NAME;

						code->event_data.push_back(FUNC::FUNC_GET_NAME);
						code->constant_data.push_back(0);
					}
					else if (name == "$find"sv) {
						token.func = FUNC::FUNC_FIND;

						code->event_data.push_back(FUNC::FUNC_FIND);
						code->constant_data.push_back(0);
					}
					else if (name == "$NOT"sv) {
						token.func = FUNC::FUNC_NOT;

						code->event_data.push_back(FUNC::FUNC_NOT);
						code->constant_data.push_back(0);
					}
					else if (name == "$is_end"sv) {
						token.func = FUNC::FUNC_IS_END;

						code->event_data.push_back(FUNC::FUNC_IS_END);
						code->constant_data.push_back(0);
					}
					else if (name == "$load_data"sv) {
						token.func = FUNC::FUNC_LOAD_DATA;

						code->event_data.push_back(FUNC::FUNC_LOAD_DATA);
						code->constant_data.push_back(0);
					}
					else if (name == "$load_json"sv) {
						token.func = FUNC::FUNC_LOAD_JSON;

						code->event_data.push_back(FUNC::FUNC_LOAD_JSON);
						code->constant_data.push_back(0);
					}
					else if (name == "$next"sv) {
						token.func = FUNC::FUNC_NEXT;

						code->event_data.push_back(FUNC::FUNC_NEXT);
						code->constant_data.push_back(0);
					}
					else if (name == "$enter"sv) {
						token.func = FUNC::FUNC_ENTER;

						code->event_data.push_back(FUNC::FUNC_ENTER);
						code->constant_data.push_back(0);
					}
					else if (name == "$quit"sv) {
						token.func = FUNC::FUNC_QUIT;

						code->event_data.push_back(FUNC::FUNC_QUIT);
						code->constant_data.push_back(0);
					}
					else if (name == "$parameter"sv) {
						for (int i = 0; i < ut->GetUserTypeList(ut_count)->GetItemListSize(); ++i) {

							auto name = (code->input)[code->constant_data.back()].ToCString();
							code->constant_data.pop_back(); // name
							code->event_data.pop_back(); // CONSTATNT

							e->parameter[name] = Token();
						}
					}
					else if (name == "$local"sv) {
						for (int i = 0; i < ut->GetUserTypeList(ut_count)->GetItemListSize(); ++i) {

							auto name = (code->input)[code->constant_data.back()].ToCString();
							code->constant_data.pop_back(); // name
							code->event_data.pop_back(); // CONSTANT

							e->local[name] = Token();
						}
					}
					else if (name == "$assign"sv) {
						token.func = FUNC::FUNC_ASSIGN;

						code->event_data.push_back(FUNC::FUNC_ASSIGN);
						code->constant_data.push_back(0);
					}
					else if (name == "$COMP<"sv) {
						token.func = FUNC::FUNC_COMP_RIGHT;

						code->event_data.push_back(FUNC::FUNC_COMP_RIGHT);
						code->constant_data.push_back(0);
					}
					else if (name == "$COMP>"sv) {
						token.func = FUNC::FUNC_COMP_LEFT;

						code->event_data.push_back(FUNC::FUNC_COMP_LEFT);
						code->constant_data.push_back(0);
					}
					else if (name == "$AND_ALL"sv) {
						token.func = FUNC::FUNC_AND_ALL;

						code->event_data.push_back(FUNC::FUNC_AND_ALL);
						code->constant_data.push_back(ut->GetUserTypeList(ut_count)->GetIListSize());
					}
					else if (name == "$AND"sv) {
						token.func = FUNC::FUNC_AND;

						code->event_data.push_back(FUNC::FUNC_AND);
						code->constant_data.push_back(0);
					}
					else if (name == "$OR"sv) {
						token.func = FUNC::FUNC_OR;

						code->event_data.push_back(FUNC::FUNC_OR);
						code->constant_data.push_back(0);
					}
					else if (name == "$get_size"sv) {
						token.func = FUNC::FUNC_GET_SIZE;

						code->event_data.push_back(FUNC::FUNC_GET_SIZE);
						code->constant_data.push_back(0);
					}
					else if (name == "$get_idx"sv) {
						token.func = FUNC::FUNC_GET_IDX;

						code->event_data.push_back(FUNC::FUNC_GET_IDX);
						code->constant_data.push_back(0);
					}
					else if (name == "$return"sv) {
						token.func = FUNC::FUNC_RETURN;

						code->event_data.push_back(FUNC::FUNC_RETURN);

						code->constant_data.push_back(ut->GetUserTypeList(ut_count)->GetIListSize());
					}
					else if (name == "$return_value"sv) {
						token.func = FUNC::FUNC_RETURN_VALUE;

						code->event_data.push_back(FUNC::FUNC_RETURN_VALUE);
						code->constant_data.push_back(0);
					}
					else if (name == "$set_name"sv) {
						token.func = FUNC::FUNC_SET_NAME;

						code->event_data.push_back(FUNC::FUNC_SET_NAME);
						code->constant_data.push_back(0);
					}
					else if (name == "$get_value"sv) {
						token.func = FUNC::FUNC_GET_VALUE;

						code->event_data.push_back(FUNC::FUNC_GET_VALUE);
						code->constant_data.push_back(0);
					}
					else if (name == "$set_value"sv) {
						token.func = FUNC::FUNC_SET_VALUE;

						code->event_data.push_back(FUNC::FUNC_SET_VALUE);
						code->constant_data.push_back(0);
					}
					else if (name == "$is_item"sv) {
						token.func = FUNC::FUNC_IS_ITEM;

						code->event_data.push_back(FUNC::FUNC_IS_ITEM);
						code->constant_data.push_back(0);
					}
					else if (name == "$is_group"sv) {
						token.func = FUNC::FUNC_IS_GROUP;

						code->event_data.push_back(FUNC::FUNC_IS_GROUP);
						code->constant_data.push_back(0);
					}
					else if (name == "$is_quoted_str"sv) {
						token.func = FUNC::FUNC_IS_QUOTED_STR;

						code->event_data.push_back(FUNC::FUNC_IS_QUOTED_STR);
						code->constant_data.push_back(0);
					}
					else if (name == "$remove_quoted"sv) {
						token.func = FUNC::FUNC_REMOVE_QUOTED;

						code->event_data.push_back(FUNC::FUNC_REMOVE_QUOTED);
						code->constant_data.push_back(0);
					}
					else if (name == "$get_now"sv) {

						token.func = FUNC::FUNC_GET_NOW;

						code->event_data.push_back(FUNC::FUNC_GET_NOW);
						code->constant_data.push_back(0);
					}
					else if (name == "$print"sv) {
						token.func = FUNC::FUNC_PRINT;

						code->event_data.push_back(FUNC::FUNC_PRINT);
						code->constant_data.push_back(ut->GetUserTypeList(ut_count)->GetIListSize());
					}

					// todo - add processing. errors..

					code->input.push_back(std::move(token));
				}
				else {
					Token token;

					token.SetString(name.c_str());

					code->input.push_back(std::move(token));
					code->event_data.push_back(FUNC::CONSTANT);
					code->constant_data.push_back(code->input.size() - 1);
				}
			}

			ut_count++;
		}
	}
}

void Debug(const EventCode& e) {
	for (int i = 0; i < e.event_data.size(); ++i) {
		if (e.event_data[i] < FUNC::SIZE) {
			std::cout << func_to_str[e.event_data[i]] << " ";
		}
		else {
			//std::cout << e.event_data[i] << " \n";
		}
	}
	std::cout << "\n";
}

// need to exception processing.
void VM::MakeByteCode(clau_parser::UserType* ut, Event& e, EventCode& code, std::string& id) {
	code.input.clear(); // = wiz::SmartPtr<std::vector<Token>>(new std::vector<Token>());

	_MakeByteCode(ut, &e, &code, &id);

	code.event_data.push_back(FUNC::FUNC_RETURN);
	code.constant_data.push_back(0);

	Debug(code);
}
