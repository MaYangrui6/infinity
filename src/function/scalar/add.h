//
// Created by JinHai on 2022/9/28.
//

#pragma once

#include "common/types/alias/smart_ptr.h"

namespace infinity {

class NewCatalog;

void RegisterAddFunction(const UniquePtr<NewCatalog> &catalog_ptr);

}
