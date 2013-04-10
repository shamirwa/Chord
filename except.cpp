#include <iostream>

using namespace std;
class U0 {};
class U1 {};

void baz(int i)
{
				if (!i) throw U0();
				else throw U1();
}
void bar(int i) { baz(i); }

void foo()
{
				for (int i = 0; i < 2; i++)
				{   
								try {
												bar(i);
												cout << " Hello am here\n";
								} catch(U0 e) {
												std::cout<< "Exception U0 caught\n";
								}
				}
}

int main() {
				foo();
				std::cout<< "Should never get here!\n";
}
