#include "GAry.hh"

int main(int argc, char** argv)
{
    GAry<float>* a = GAry<float>::from_constant(10, 1.f) ;
    a->Summary("a");

    GAry<float>* c = a->cumsum();
    c->Summary("a->cumsum()");


    return 0 ;
}
