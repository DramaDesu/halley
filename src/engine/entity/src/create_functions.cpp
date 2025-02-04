#include <halley/entity/create_functions.h>
using namespace Halley;

CreateComponentFunction& CreateEntityFunctions::getCreateComponent()
{
	static CreateComponentFunction f;
	return f;
}

CreateSystemFunction& CreateEntityFunctions::getCreateSystem()
{
	static CreateSystemFunction f;
	return f;
}

CreateMessageFunction& CreateEntityFunctions::getCreateMessage()
{
	static CreateMessageFunction f;
	return f;
}

CreateMessageByNameFunction& CreateEntityFunctions::getCreateMessageByName()
{
	static CreateMessageByNameFunction f;
	return f;
}

CreateSystemMessageFunction& CreateEntityFunctions::getCreateSystemMessage()
{
	static CreateSystemMessageFunction f;
	return f;
}

CreateSystemMessageByNameFunction& CreateEntityFunctions::getCreateSystemMessageByName()
{
	static CreateSystemMessageByNameFunction f;
	return f;
}
