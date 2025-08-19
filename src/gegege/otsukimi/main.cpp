#include <gegege/otsukimi/otsukimi.hpp>

int main(int argc, char** argv)
{
    gegege::otsukimi::Otsukimi otsukimi;
    otsukimi.startup();
    otsukimi.shutdown();
    return 0;
}
