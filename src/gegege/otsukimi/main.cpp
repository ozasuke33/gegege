#include <gegege/otsukimi/otsukimi.hpp>

int main(int argc, char** argv)
{
    gegege::otsukimi::Otsukimi otsukimi;
    otsukimi.startup();
    otsukimi.run();
    otsukimi.shutdown();
    return 0;
}
