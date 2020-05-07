#ifndef __CONFIG_H__
#define	__CONFIG_H__

#include <string>
#include <iostream>
#include <memory>
#include <fstream>
#include "json/json.h"

namespace fb {

const Json::Value& config();

}

#endif // !__CONFIG_H__
