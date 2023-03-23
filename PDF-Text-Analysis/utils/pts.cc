#include "PopplerWrapper.h"

int main(int argc, char *argv[]) {

    setlocale(LC_ALL, "");
    std::wcin.imbue(std::locale("en_US.UTF-8"));
    std::wcerr.imbue(std::locale("en_US.UTF-8"));
    std::wcout.imbue(std::locale("en_US.UTF-8"));
    std::wcout.precision(14);

    if (argc != 3) {
        std::cerr << "USAGE: {input PDF} {page #}" << std::endl;
        exit(1);
    }

    PopplerWrapper *wrap = new PopplerWrapper(argv[1]);
    wrap->setPageNumber(std::stoi(argv[2]));

    auto infos = wrap->getTextInfo();

    wrap->print_infos(infos);
}
