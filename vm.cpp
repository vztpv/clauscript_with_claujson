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
		token.SetString({});
		return { token };
	}
	else {
		auto x = clau_parser::UserType::Find(ut, str.substr(0, idx + 1));

		if (x.first == false) { return result; }

		for (int i = 0; i < x.second.size(); ++i) {
			std::string itemName = str.substr(idx + 1);

			if (itemName.starts_with("%it") && itemName.size() >= 4) {
				int itemIdx = std::stoi(itemName.substr(3));

				Token temp;
				temp.SetString(x.second[i]->GetItemList(itemIdx).Get(0));
				result.push_back(temp);
			}
			else {
				if (itemName == "_") {
					itemName = "";
				}
				auto temp = x.second[i]->GetItem(itemName);
				if (!temp.empty()) {
					for (int j = 0; j < temp.size(); ++j) {
						Token tkn;
						tkn.SetString(temp[j].Get(0));

						result.push_back(tkn);
					}
				}
			}
		}
	}
	return result;
}


void _MakeByteCode(clau_parser::UserType* ut, Event* e) {
	long long it_count = 0, ut_count = 0;

	for (long long i = 0; i < ut->GetIListSize(); ++i) {
		if (ut->IsItemList(i)) {
			if (!ut->GetItemList(it_count).GetName().empty()) {
				if (ut->GetItemList(it_count).GetName() == "id"sv && ut->GetName() == "Event"sv) {
					it_count++;
					continue;
				}

				{
					Token token(ut);
					token.SetString(ut->GetItemList(it_count).GetName());

					e->input->push_back(token);
				}

				e->event_data.push_back(FUNC::CONSTANT);
				e->event_data.push_back(e->input->size() - 1);

				{
					auto a = ut->GetItemList(it_count).Get();

					if (a.starts_with("/")) {

						e->event_data.push_back(FUNC::START_DIR);

						auto tokens = clau_parser::tokenize(a, '/');

						for (int i = 0; i < tokens.size(); ++i) {
							if (tokens[i].starts_with("$"sv) && !tokens[i].starts_with("$parameter."sv) && !tokens[i].starts_with("$local."sv)) {
								clau_parser::UserType new_ut;

								new_ut.AddUserTypeItem(clau_parser::UserType(tokens[i]));

								_MakeByteCode(&new_ut, e);
							}
							else {
								Token token(ut);
								token.SetString(tokens[i]);

								e->input->push_back(token);
								e->event_data.push_back(FUNC::CONSTANT);
								e->event_data.push_back(e->input->size() - 1);
								e->event_data.push_back(FUNC::DIR);
							}
						}

						e->event_data.push_back(FUNC::END_DIR);
					}
					else if (a.starts_with("@")) {
						auto tokens = clau_parser::tokenize(a, '@');

						for (int i = 0; i < tokens.size(); ++i) {
							if (tokens[i].starts_with("$"sv) && !tokens[i].starts_with("$parameter."sv) && !tokens[i].starts_with("$local."sv)) {
								clau_parser::UserType new_ut;

								new_ut.AddUserTypeItem(clau_parser::UserType(tokens[i]));

								_MakeByteCode(&new_ut, e);
								break;
							}
							else {
								Token token(ut);
								token.SetString(tokens[i]);

								e->input->push_back(token);
								e->event_data.push_back(FUNC::CONSTANT);
								e->event_data.push_back(e->input->size() - 1);
							}
						}
					}
					else {
						Token token;

						token.SetString(ut->GetItemList(it_count).Get());

						e->input->push_back(token);

						e->event_data.push_back(FUNC::CONSTANT);
						e->event_data.push_back(e->input->size() - 1);
					}
				}
			}

			// $while, $if
			else if (ut->GetItemList(it_count).GetName().empty()) {
				if (ut->GetItemList(it_count).Get() == "$while"sv) {
					e->event_data.push_back(FUNC::FUNC_WHILE);
					int idx = e->event_data.size() - 1;
					int then_idx = 0;

					_MakeByteCode(ut->GetUserTypeList(ut_count), e);

					ut_count++; ++i;

					e->event_data.push_back(FUNC::THEN);
					e->event_data.push_back(0);
					then_idx = e->event_data.size() - 1;
					{
						//Event _e;
						//int count2 = _MakeByteCode(ut->GetUserTypeList(ut_count), &_e);
						//count2;
						//e->event_data.push_back(0); 
					}

					_MakeByteCode(ut->GetUserTypeList(ut_count), e);

					ut_count++; ++i;

					e->event_data.push_back(FUNC::WHILE_END);
					e->event_data.push_back(idx);

					e->event_data[then_idx] = e->event_data.size(); //
				}
				else if (ut->GetItemList(it_count).Get() == "$if"sv) {
					e->event_data.push_back(FUNC::FUNC_IF);
					int idx = 0;

					_MakeByteCode(ut->GetUserTypeList(ut_count), e);
					ut_count++; ++i;

					e->event_data.push_back(FUNC::THEN);
					e->event_data.push_back(0);
					idx = e->event_data.size() - 1;

					_MakeByteCode(ut->GetUserTypeList(ut_count), e);

					ut_count++; ++i;

					e->event_data.push_back(FUNC::IF_END);
					e->event_data[idx] = e->event_data.size();
				}
				else if (ut->GetItemList(it_count).Get() == "TRUE"sv) {
					e->event_data.push_back(FUNC::TRUE);
				}
				else if (ut->GetItemList(it_count).Get() == "FALSE"sv) {
					e->event_data.push_back(FUNC::FALSE);
				}
				else {
					auto a = ut->GetItemList(it_count).Get();

					if (a.starts_with("/")) {

						e->event_data.push_back(FUNC::START_DIR);

						auto tokens = clau_parser::tokenize(a, '/');

						for (int i = 0; i < tokens.size(); ++i) {
							if (tokens[i].starts_with("$"sv) && !tokens[i].starts_with("$parameter."sv) && !tokens[i].starts_with("$local."sv)) {
								clau_parser::UserType new_ut;

								new_ut.AddUserTypeItem(clau_parser::UserType(tokens[i]));

								_MakeByteCode(&new_ut, e);
								break;
							}
							else {
								Token token(ut);

								token.SetString(tokens[i]);

								e->input->push_back(token);
								e->event_data.push_back(FUNC::CONSTANT);
								e->event_data.push_back(e->input->size() - 1);
								e->event_data.push_back(FUNC::DIR);
							}
						}

						e->event_data.push_back(FUNC::END_DIR);
					}
					else if (a.starts_with("@")) {
						auto tokens = clau_parser::tokenize(a, '@');

						for (int i = 0; i < tokens.size(); ++i) {
							if (tokens[i].starts_with("$"sv) && !tokens[i].starts_with("$parameter."sv) && !tokens[i].starts_with("$local."sv)) {
								clau_parser::UserType new_ut;

								new_ut.AddUserTypeItem(clau_parser::UserType(tokens[i]));

								_MakeByteCode(&new_ut, e);
							}
							else {
								Token token;

								token.SetString(tokens[i]);

								e->input->push_back(token);
								e->event_data.push_back(FUNC::CONSTANT);
								e->event_data.push_back(e->input->size() - 1);
							}
						}
					}
					else {
						Token token;

						token.SetString(ut->GetItemList(it_count).Get());

						e->input->push_back(token);

						e->event_data.push_back(FUNC::CONSTANT);
						e->event_data.push_back(e->input->size() - 1);
					}
				}
			}

			it_count++;
		}
		else {
			std::string name = ut->GetUserTypeList(ut_count)->GetName();
			bool call_flag = false;

			if (name == "$call"sv) {
				call_flag = true;
			}

			if (name != "$query"sv && name != "$search"sv) {
				_MakeByteCode(ut->GetUserTypeList(ut_count), e);
			}

			if (!ut->GetUserTypeList(ut_count)->GetName().empty()) {
				if (name.starts_with("$"sv)) {
					Token token;

					token.SetFunc(); // | Token::Type::UserType

					if (name == "$is_int"sv) {
						token.func = FUNC::FUNC_IS_INT;

						e->event_data.push_back(FUNC::FUNC_IS_INT);
					}
					else if (name == "$is_float"sv) {
						token.func = FUNC::FUNC_IS_FLOAT;

						e->event_data.push_back(FUNC::FUNC_IS_FLOAT);
					}
					else if (name == "$split"sv) {
						token.func = FUNC::FUNC_SPLIT;

						e->event_data.push_back(FUNC::FUNC_SPLIT);
					}
					else if (name == "$make_global"sv) {
						token.func = FUNC::FUNC_MAKE_GLOBAL;

						e->event_data.push_back(FUNC::FUNC_MAKE_GLOBAL);
					}
					else if (name == "$clear_global"sv) {
						token.func = FUNC::FUNC_CLEAR_GLOBAL;

						e->event_data.push_back(FUNC::FUNC_CLEAR_GLOBAL);
					}
					else if (name == "$remove"sv) {
						token.func = FUNC::FUNC_REMOVE;

						e->event_data.push_back(FUNC::FUNC_REMOVE);
					}
					else if (name == "$count_global"sv) {
						token.func = FUNC::FUNC_COUNT_GLOBAL;

						e->event_data.push_back(FUNC::FUNC_COUNT_GLOBAL);
					}
					else if (name == "$search"sv) {
						_MakeByteCode(ut->GetUserTypeList(ut_count)->GetUserTypeList(0), e);
						_MakeByteCode(ut->GetUserTypeList(ut_count)->GetUserTypeList(1), e);

						token.func = FUNC::FUNC_SEARCH;

						e->event_data.push_back(FUNC::FUNC_SEARCH);

						{
							Token temp;
							temp.ut_val = wiz::SmartPtr<clau_parser::UserType>(new clau_parser::UserType(*ut->GetUserTypeList(ut_count)));

							e->input->push_back(temp);
							e->event_data.push_back(e->input->size() - 1);
						}
					}
					else if (name == "$query"sv) {
						_MakeByteCode(ut->GetUserTypeList(ut_count)->GetUserTypeList(0), e);

						token.func = FUNC::FUNC_QUERY;

						e->event_data.push_back(FUNC::FUNC_QUERY);

						{
							Token temp;
							temp.ut_val = wiz::SmartPtr<clau_parser::UserType>(new clau_parser::UserType(*ut->GetUserTypeList(ut_count)));

							e->input->push_back(temp);
							e->event_data.push_back(e->input->size() - 1);
						}
					}
					else if (name == "$clone"sv) {
						token.func = FUNC::FUNC_CLONE;

						e->event_data.push_back(FUNC::FUNC_CLONE);
					}
					else if (name == "$call"sv) {
						token.func = FUNC::FUNC_CALL;

						e->event_data.push_back(FUNC::FUNC_CALL);
						e->event_data.push_back(ut->GetUserTypeList(ut_count)->GetItemListSize());

					}
					else if (name == "$not_empty"sv) {
						token.func = FUNC::FUNC_NOT_EMPTY;
						e->event_data.push_back(FUNC::FUNC_NOT_EMPTY);
					}
					else if (name == "$set_idx"sv) {
						token.func = FUNC::FUNC_SET_IDX;

						e->event_data.push_back(FUNC::FUNC_SET_IDX);
					}
					else if (name == "$add"sv) {
						token.func = FUNC::FUNC_ADD;

						e->event_data.push_back(FUNC::FUNC_ADD);
					}
					else if (name == "$get"sv) {
						token.func = FUNC::FUNC_GET;

						e->event_data.push_back(FUNC::FUNC_GET);
					}
					else if (name == "$set_global"sv) {
						token.func = FUNC::FUNC_SET_GLOBAL;
						e->event_data.push_back(FUNC::FUNC_SET_GLOBAL);
					}
					else if (name == "$get_global"sv) {
						token.func = FUNC::FUNC_GET_GLOBAL;

						e->event_data.push_back(FUNC::FUNC_GET_GLOBAL);
					}
					else if (name == "$get_name"sv) {
						token.func = FUNC::FUNC_GET_NAME;

						e->event_data.push_back(FUNC::FUNC_GET_NAME);
					}
					else if (name == "$find"sv) {
						token.func = FUNC::FUNC_FIND;

						e->event_data.push_back(FUNC::FUNC_FIND);
					}
					else if (name == "$NOT"sv) {
						token.func = FUNC::FUNC_NOT;

						e->event_data.push_back(FUNC::FUNC_NOT);
					}
					else if (name == "$is_end"sv) {
						token.func = FUNC::FUNC_IS_END;

						e->event_data.push_back(FUNC::FUNC_IS_END);
					}
					else if (name == "$load_data"sv) {
						token.func = FUNC::FUNC_LOAD_DATA;

						e->event_data.push_back(FUNC::FUNC_LOAD_DATA);
					}
					else if (name == "$load_json"sv) {
						token.func = FUNC::FUNC_LOAD_JSON;

						e->event_data.push_back(FUNC::FUNC_LOAD_JSON);
					}
					else if (name == "$next"sv) {
						token.func = FUNC::FUNC_NEXT;

						e->event_data.push_back(FUNC::FUNC_NEXT);
					}
					else if (name == "$enter"sv) {
						token.func = FUNC::FUNC_ENTER;

						e->event_data.push_back(FUNC::FUNC_ENTER);
					}
					else if (name == "$quit"sv) {
						token.func = FUNC::FUNC_QUIT;

						e->event_data.push_back(FUNC::FUNC_QUIT);
					}
					else if (name == "$parameter"sv) {
						for (int i = 0; i < ut->GetUserTypeList(ut_count)->GetItemListSize(); ++i) {

							auto name = (*e->input)[e->event_data.back()].ToString();
							e->event_data.pop_back(); // name
							e->event_data.pop_back(); // CONSTATNT

							e->parameter[name] = Token();
						}
					}
					else if (name == "$local"sv) {
						for (int i = 0; i < ut->GetUserTypeList(ut_count)->GetItemListSize(); ++i) {

							auto name = (*e->input)[e->event_data.back()].ToString();
							e->event_data.pop_back(); // name
							e->event_data.pop_back(); // CONSTANT

							e->local[name] = Token();
						}
					}
					else if (name == "$assign"sv) {
						token.func = FUNC::FUNC_ASSIGN;

						e->event_data.push_back(FUNC::FUNC_ASSIGN);
					}
					else if (name == "$COMP<"sv) {
						token.func = FUNC::FUNC_COMP_RIGHT;

						e->event_data.push_back(FUNC::FUNC_COMP_RIGHT);
					}
					else if (name == "$COMP>"sv) {
						token.func = FUNC::FUNC_COMP_LEFT;

						e->event_data.push_back(FUNC::FUNC_COMP_LEFT);
					}
					else if (name == "$AND_ALL"sv) {
						token.func = FUNC::FUNC_AND_ALL;

						e->event_data.push_back(FUNC::FUNC_AND_ALL);
						e->event_data.push_back(ut->GetUserTypeList(ut_count)->GetIListSize());
					}
					else if (name == "$AND"sv) {
						token.func = FUNC::FUNC_AND;

						e->event_data.push_back(FUNC::FUNC_AND);
					}
					else if (name == "$OR"sv) {
						token.func = FUNC::FUNC_OR;

						e->event_data.push_back(FUNC::FUNC_OR);
					}
					else if (name == "$get_size"sv) {
						token.func = FUNC::FUNC_GET_SIZE;

						e->event_data.push_back(FUNC::FUNC_GET_SIZE);
					}
					else if (name == "$get_idx"sv) {
						token.func = FUNC::FUNC_GET_IDX;

						e->event_data.push_back(FUNC::FUNC_GET_IDX);
					}
					else if (name == "$return"sv) {
						token.func = FUNC::FUNC_RETURN;

						e->event_data.push_back(FUNC::FUNC_RETURN);

						e->event_data.push_back(ut->GetUserTypeList(ut_count)->GetIListSize());
					}
					else if (name == "$return_value"sv) {
						token.func = FUNC::FUNC_RETURN_VALUE;

						e->event_data.push_back(FUNC::FUNC_RETURN_VALUE);
					}
					else if (name == "$set_name"sv) {
						token.func = FUNC::FUNC_SET_NAME;

						e->event_data.push_back(FUNC::FUNC_SET_NAME);
					}
					else if (name == "$get_value"sv) {
						token.func = FUNC::FUNC_GET_VALUE;

						e->event_data.push_back(FUNC::FUNC_GET_VALUE);
					}
					else if (name == "$set_value"sv) {
						token.func = FUNC::FUNC_SET_VALUE;

						e->event_data.push_back(FUNC::FUNC_SET_VALUE);
					}
					else if (name == "$is_item"sv) {
						token.func = FUNC::FUNC_IS_ITEM;

						e->event_data.push_back(FUNC::FUNC_IS_ITEM);
					}
					else if (name == "$is_group"sv) {
						token.func = FUNC::FUNC_IS_GROUP;

						e->event_data.push_back(FUNC::FUNC_IS_GROUP);
					}
					else if (name == "$is_quoted_str"sv) {
						token.func = FUNC::FUNC_IS_QUOTED_STR;

						e->event_data.push_back(FUNC::FUNC_IS_QUOTED_STR);
					}
					else if (name == "$remove_quoted"sv) {
						token.func = FUNC::FUNC_REMOVE_QUOTED;

						e->event_data.push_back(FUNC::FUNC_REMOVE_QUOTED);
					}
					else if (name == "$get_now"sv) {

						token.func = FUNC::FUNC_GET_NOW;

						e->event_data.push_back(FUNC::FUNC_GET_NOW);
					}
					else if (name == "$print"sv) {
						token.func = FUNC::FUNC_PRINT;

						e->event_data.push_back(FUNC::FUNC_PRINT);
						e->event_data.push_back(ut->GetUserTypeList(ut_count)->GetIListSize());
					}

					// todo - add processing. errors..

					e->input->push_back(token);
				}
				else {
					Token token;

					token.SetString(std::move(name));

					e->input->push_back(token);
					e->event_data.push_back(FUNC::CONSTANT);
					e->event_data.push_back(e->input->size() - 1);
				}
			}

			ut_count++;
		}
	}
}

void Debug(const Event& e) {
	for (int i = 0; i < e.event_data.size(); ++i) {
		if (e.event_data[i] < FUNC::SIZE) {
			//std::cout << func_to_str[e.event_data[i]] << " ";
		}
		else {
			//std::cout << e.event_data[i] << " \n";
		}
	}
}

// need to exception processing.
Event MakeByteCode(clau_parser::UserType* ut) {
	Event e;
	e.input = wiz::SmartPtr<std::vector<Token>>(new std::vector<Token>());

	_MakeByteCode(ut, &e);


	e.event_data.push_back(FUNC::FUNC_RETURN);
	e.event_data.push_back(0);

	e.id = ut->GetItem("id")[0].Get();

	Debug(e);

	return e;
}
