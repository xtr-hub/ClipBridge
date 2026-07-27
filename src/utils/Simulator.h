#include<QString>

namespace ClipBridge {

class Simulator
{
public:
    /**
    * @brief 模拟粘贴按键
    * @param keySequence 按键序列，如 "Ctrl+V"、"Ctrl+Shift+V"、"Shift+Insert"
    */
    static void simulatePaste(const QString &keySequence = "Ctrl+V");

    /**
    * @brief 模拟回车键 (Enter/Return)，用于自动提交
    */
    static void simulateEnter();
};

}