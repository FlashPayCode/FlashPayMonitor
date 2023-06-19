#include "public.h"

std::map<ReturnCode, const char *> ReturnMsg::RETURN_MSG = {
        {ReturnCode ::ERROR_JSON, "Json wrong format."},
        {ReturnCode ::ERROR_ADD_DATA, "Add data fail."},
        {ReturnCode ::ERROR_API_NONEXISTENCE, "API does not exist."},
        {ReturnCode ::COMPLETE, "Success."},

};


const char *ReturnMsg::getReturnMsg(ReturnCode returnCode) {
    return RETURN_MSG[returnCode];
}
