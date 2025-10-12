

#include "mimalloc-new-delete.h" // comment?

#include <iostream>
#include <ctime>

#include "vm.h"

int main(void)
{
	VM vm;

	clau_parser::UserType global;
	clau_parser::LoadData::LoadDataFromFile("test.txt", global, 1, 0);

	auto arr = global.GetUserTypeIdx("Event");
	std::vector<std::string> ids;
	ids.resize(arr.size());
	int64_t count = 0;

	for (auto x : arr) {
		Event e;
		EventCode code;
		std::string& id = ids[count]; ++count;
		vm.MakeByteCode(global.GetUserTypeList(x), e, code, id);
		std::cout << id << "\n";
		vm.Register(id, std::move(e), std::move(code));
	}

	for (auto& x : ids) {
		std::cout << x << " ";
	}
	int start = clock();
	vm.Run("main", &global);
	int last = clock();
	std::cout << last - start << "ms\n";
	
	//clau_parser::LoadData::Save(global, "output.eu4");

	return 0;
}

