#include "Server.h"

using namespace RestAPIServer;

int main(int argc, char** argv)
{
	Start("127.0.0.1", 55555, "./ServerRoot");
	return 0;
}