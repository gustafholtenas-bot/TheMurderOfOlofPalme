#include "../../Plugins/TMOPEngine/Source/TMOPEngine/Private/World/TMOPInformationTimeWindow.h"
#include <iostream>

int main()
{
    struct Case { const char* Name; int Now; int Start; int End; bool Expected; };
    const Case Cases[] = {
        {"before party", 82799, 82800, 85500, false},
        {"party starts", 82800, 82800, 85500, true},
        {"party underway", 83700, 82800, 85500, true},
        {"last second", 85499, 82800, 85500, true},
        {"party ends", 85500, 82800, 85500, false},
        {"after party", 86100, 82800, 85500, false},
        {"seek backwards into party", 83700, 82800, 85500, true},
        {"loop restart before party", 82800, 83400, 85500, false},
        {"midnight window before start", 82799, 82800, 1800, false},
        {"midnight window start", 82800, 82800, 1800, true},
        {"midnight window midnight", 0, 82800, 1800, true},
        {"midnight window last second", 1799, 82800, 1800, true},
        {"midnight window end", 1800, 82800, 1800, false},
        {"midnight window daytime", 43200, 82800, 1800, false},
        {"equal endpoints are empty", 82800, 82800, 82800, false},
        {"negative clock", -1, 82800, 85500, false},
        {"clock outside day", 86400, 82800, 85500, false},
        {"invalid start", 83700, -1, 85500, false},
        {"invalid end", 83700, 82800, 86400, false},
    };
    int Failed = 0;
    for (const auto& Test : Cases)
        if (TMOPInformationTimeWindow::Contains(Test.Now, Test.Start, Test.End) != Test.Expected)
        {
            std::cerr << "FAIL: " << Test.Name << '\n';
            ++Failed;
        }
    if (Failed) return 1;
    std::cout << sizeof(Cases) / sizeof(Case) << " information time-window cases passed\n";
    return 0;
}
