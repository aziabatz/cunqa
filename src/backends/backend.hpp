#include <iostream>
#include <fstream>
#include <memory>
#include <optional>

#include "utils/json.hpp"

namespace cunqa {

class Backend {
public:
    virtual Backend();
    virtual ~Backend();

    virtual JSON execute(JSON& circuit_json);
};

}