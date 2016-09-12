#include "wxconfig.h"
#include <QCoreApplication>
WxConfig::WxConfig()
{
    WXPAY_WORKDIR = QCoreApplication::applicationDirPath();
    PROXY_IP = "127.0.0.1";
    PROXY_PORT = 8888;
    TEST_URL = "https://api.mch.weixin.qq.com/pay/orderquery";
}

