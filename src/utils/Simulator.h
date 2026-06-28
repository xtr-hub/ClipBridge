namespace ClipBridge {

class Simulator
{
public:
    /**
    * @brief 模拟粘贴按键 (Ctrl+V)
    */
    static void simulatePaste();

    /**
    * @brief 模拟回车键 (Enter/Return)，用于自动提交
    */
    static void simulateEnter();
};

}