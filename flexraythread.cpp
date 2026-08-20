#include "flexraythread.h"
#include "applogger.h"
#include <QElapsedTimer>
#include <QMetaObject>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>

//初始化flexray总线的cluster参数
FlexrayClusterParameter_t mFlexrayClusterParameter{
        10,  //uint8_t gColdstartAttempts;               /*!< 允许簇内节点通过启动调度同步尝试启动簇的最大次数，有效值为 2~31 次 */
        7,   //uint8_t gdActionPointOffset;              /*!< 动作点偏离静态时隙或符号窗的起始点的宏节拍数量，有效值为 1~63MT */
        91,  //uint8_t gdCASRxLowMax;                    /*!< CAS 接收窗口上限，有效值为 67~99gdBit */
        1,   //uint8_t gdDynamicSlotIdlePhase;           /*!< 一个动态时隙内的空闲阶段持续时间，有效值为 0~2 */
        6,  //uint8_t gdMinislot;              ··         /*!< 微时隙的持续时间，有效值为 2~63MT */
        2,   //uint8_t gdMinislotActionPointOffset;      /*!< 微时隙动作点偏离微时隙起始点的宏节拍数量，有效值为 1~31MT。 */
        50, //uint16_t gdStaticSlot;                     /*!< 静态时隙的持续时间，有效值为 4~661MT */
        0,   //uint8_t gdSymbolWindow;                   /*!< 符号窗的持续时间，有效值为 0~142M  */
        11,  //uint8_t gdTSSTransmitter;                 /*!< 传输起始序列（TSS）的位数，有效值为 3~15gdBit */
        59,  //uint8_t gdWakeupSymbolRxIdle;             /*!< 节点对收到唤醒符号的"空闲"部分持续时间进行测试时所使用的位数。持续时间 等于(gdWakeupSymbolTxIdle-gdWakeupSymbolTxLow)/2 再减去一个安全部分，有效值为 14~59gdBit */
        50,  //uint8_t gdWakeupSymbolRxLow;              /*!< 节点对收到唤醒符号的 LOW 部分进行测试时所使用的位数。这是接收器为检测LOW 部分必须接收到的零的位数。该持续时间等于 gdWakeupSymbolTxLow 减去一个安全部分，有效值为 11~59gdBit */
        301, //uint16_t gdWakeupSymbolRxWindow;          /*!< 检测唤醒所使用的窗口的大小。唤醒检测需要一段 LOW 和空闲时间(从一个 WUS)，在这个大小的窗口内，能够完整检测一段 LOW 的时间(另一个 WUS)。这个持续时间等于 gdWakeupSymbolTxIdle+2×gdWakeupSymbolTxLow，再加上一个安全部分，有效值为 76~301gdBit。 */
        180, //uint8_t gdWakeupSymbolTxIdle;             /*!< 节点发送唤醒符的"空闲"部分所使用的位数，持续时间等于 18μs，有效值为45~180gdBit。 */
        60,  //uint8_t gdWakeupSymbolTxLow;              /*!< 节点发送唤醒符的"LOW"部分所使用的位数。持续时间等于 6μs，有效值为15~60gdBit。 */
        2,   //uint32_t gListenNoise;                    /*!< 在存在噪声的情况下，启动监听超时值和唤醒监听超时值的上限。它是节点参数pdListenTimeout 的倍数，有效值为 2~16 */
        0,   //uint8_t gNetworkManagementVectorLength;   /*!< 网络管理向量的长度，有效值为 0~12 字节 */
        5000,//uint16_t gMacroPerCycle;                  /*!< 一个通信周期的总宏节拍数量，有效值为 10~16000MT */
        8,   //uint8_t gMaxWithoutClockCorrectionFatal;  /*!< 定义缺失时钟修正项的连续偶/奇循环对数目，缺失时钟修正项将导致协议从正常有源状态或正常无源状态过渡到停止状态，有效值为 1~15 个偶/奇循环对 */
        8,   //uint8_t gMaxWithoutClockCorrectionPassive;/*!< 定义缺失时钟修正项的连续偶/奇循环对数目，缺失时钟修正项将导致协议从正常 有 源 状 态 过 渡 到 正 常 无 源 状 态 。 需 要 注 意 的 是gMaxWithoutClockCorrectionPassive≤gMaxWithoutClockCorrectionFatal，有效值为 1~15 个偶/奇循环对 */
        0,   //uint16_t gNumberOfMinislots;             /*!< 动态段的微时隙数，有效值为 0~7986 */
        98,  //uint16_t gNumberOfStaticSlots;            /*!< 静态段的静态时隙数，有效值为 2~1023 个 */
        4920,//uint16_t gOffsetCorrectionStart;          //!< NIT 内偏差修正相位的起点，表示周期开始后的第几个宏节拍开始修正，有效值9~15999MT
                                                         // 表示从周期开始的宏时钟数（9 - 15999 MT）
                                                         // 请注意，该值应大于
                                                         // gdStaticSlot * gNumberOfStaticSlots + gdMinislot * gNumberOfMinislots + gdSymbolWindow */
        10,  //uint8_t gPayloadLengthStatic;             /*!< 静态帧的有效负载长度，有效值为 0~127 */
        10,   //uint8_t gSyncNodeMax;                    /*!< 可发送"同步帧指示位设置为 1"的帧的最大节点数量，有效值为 2~15 个 */

};

//初始化flexray总线的node参数
FlexrayNodeParameter_t mFlexrayNodeParameter{
        401202,   //uint32_t pdListenTimeout;               /*!< 启动监听超时和唤醒监听超时值，有效值为 1284~1283846μ */
        9,        //uint8_t pMacroInitialOffsetA;           /*!< A 通道静态时隙边界和第二个时间基准点的下一个宏节拍边界之间的宏节拍数，有效值为 2~68M */
        9,        //uint8_t pMacroInitialOffsetB;           /*!< B 通道静态时隙边界和第二个时间基准点的下一个宏节拍边界之间的宏节拍数，有效值为 2~68MT */
        127,       //uint8_t pPayloadLengthDynMax;           /*!< 动态帧的最大有效负载长度，有效值为 0~127 个字 */
        1,        //uint8_t pAllowHaltDueToClock;			/*!< 允许时钟同步错误过渡到停止状态标志位，有效值为 0 和 1，1 为允许。 */
        20,        //uint8_t pAllowPassiveToActive;          /*!< 在正常无源状态到正常有源状态的过渡之前，必须要有的有效时钟修正项的连续偶/奇循环对数量。设置为 0 表示不允许从正常无源状态过渡到正常有源状态，有效值为 0~31 */
        2,        //uint8_t pClusterDriftDamping;           /*!< 本地用于速率修正的簇漂移阻尼系数，有效值为 0~20μT */
        212,      //uint16_t pdAcceptedStartupRange;        /*!< 集成过程中，启动帧所允许的、经扩展的测量时钟偏差范围，有效值为0~1875μT */
        1,        //uint16_t pDelayCompensationA;           /*!< 用来补偿 A 通道接收延迟的值。它覆盖了高达 2.5μs 的假定传播延迟，有效值为 0~200μT
        1,        //uint16_t pDelayCompensationB;           /*!< 用来补偿 B 通道接收延迟的值。它覆盖了高达 2.5μs 的假定传播延迟，有效值为 0~200μT
        //这个keyslot收发的时候，这个参数不能一致
        20,        //uint16_t pKeySlotId;                    /*!< 用于发送启动帧、同步帧或指定单时隙帧的时隙标识符（ID），有效值为 1~1023时隙。 */
        1,        //uint8_t pKeySlotUsedForStartup;         /*!< 指 示 关 键 时 隙 是 否 用 于 发 送 启 动 帧 的 标 志 ， 有 效 值 为 0 和 1 ， 若pKeySlotUsedForStartup 设置为 1，则 */
        1,        //uint8_t pKeySlotUsedForSync;            /*!< 指 示 关 键 时 隙 是 否 用 于 发 送 同 步 帧 的 标 志 ， 有 效 值 为 0 和 1 ， 若pKeySlotUsedForStartup 设置为 1，则 pKeySlotUsedForSync */
        21,       //uint16_t pLatestTx;                     /*!< 在动态段最后可以开始帧发送的微时隙数，有效值为 0~7980 */
        23,       //uint8_t pMicroInitialOffsetA;           /*!< 由 pMacroInitialOffset[CH]描述的最接近的宏节拍边界和第二时间基准点之间的微节拍数，有效值为 0~239μT。
        23,       //uint8_t pMicroInitialOffsetB;           /*!< 由 pMacroInitialOffset[CH]描述的最接近的宏节拍边界和第二时间基准点之间的微节拍数，有效值为 0~239μT
        200000,   //uint32_t pMicroPerCycle;                /*!< 一个通信周期的总微节拍数量。需要注意的是 pMicroPerCycle= gMacroPerCycle* pMicroPerMacroNom，有效值为 640~640000μT */
        601,      //uint32_t pdMaxDrift;                    /*!< 两个节点以非同步时钟运行一个通信周期以上时，两者之间的最大漂移偏差，有效值为 2~1923μT
        40,       //uint8_t pMicroPerMacroNom;              /*!< 一个宏节拍包含的微节拍数量，有效值为 40~240μT
        601,      //uint16_t pRateCorrectionOut;            /*!< 允许最大速率修正值的大小，有效值为 2~1923μT。 */
        141,      //uint16_t pOffsetCorrectionOut;          /*!< 允许最大偏差修正值的大小，有效值为 2~1923μT */
        0,        //uint8_t pSingleSlotEnabled;             //!< 启动后节点是否将进入单时隙模式标志位，有效值为 0 和 1
        //FR_CHANNEL_A
        2,        //uint8_t pWakeupChannel;                 //!< 节点用于发送唤醒模式的通道，0 为 A 通道，1 为 B 通道
        33,       //uint8_t pWakeupPattern;                 /*!< 当节点进入唤醒发送状态时，形成唤醒模式的唤醒符号重复次数，有效值为 2~63次。 */
        56,       //uint16_t pDecodingCorrection;           /*!< 接收器用于计算主时间基准点和次级时间基准点之间差异的值，有效值为14~143μT。
        0,        //uint16_t keySlotHeaderCrc;              /*!< 关键帧头 CRC 校验和，应用本模块时填写 0 即可，实际校验和由设备软件自动计算*/
        0,        //uint8_t pExternOffsetCorrection;        /*!< 进行主机请求的外部偏差修正时，NIT 加上或减去的微节拍数，有效值为 0~7μT */
        0         //uint8_t pExternRateCorrection;          /*!< 进行主机请求的外部速率修正时，周期加上或减去的微节拍数，有效值为0~7μT */


};
//初始化flexray总线的node参数
FlexrayNodeParameter_t mFlexrayNodeParameter_2{
        401202,   //uint32_t pdListenTimeout;               /*!< 启动监听超时和唤醒监听超时值，有效值为 1284~1283846μ */
        9,        //uint8_t pMacroInitialOffsetA;           /*!< A 通道静态时隙边界和第二个时间基准点的下一个宏节拍边界之间的宏节拍数，有效值为 2~68M */
        9,        //uint8_t pMacroInitialOffsetB;           /*!< B 通道静态时隙边界和第二个时间基准点的下一个宏节拍边界之间的宏节拍数，有效值为 2~68MT */
        127,       //uint8_t pPayloadLengthDynMax;           /*!< 动态帧的最大有效负载长度，有效值为 0~127 个字 */
        1,        //uint8_t pAllowHaltDueToClock;			/*!< 允许时钟同步错误过渡到停止状态标志位，有效值为 0 和 1，1 为允许。 */
        20,        //uint8_t pAllowPassiveToActive;          /*!< 在正常无源状态到正常有源状态的过渡之前，必须要有的有效时钟修正项的连续偶/奇循环对数量。设置为 0 表示不允许从正常无源状态过渡到正常有源状态，有效值为 0~31 */
        2,        //uint8_t pClusterDriftDamping;           /*!< 本地用于速率修正的簇漂移阻尼系数，有效值为 0~20μT */
        212,      //uint16_t pdAcceptedStartupRange;        /*!< 集成过程中，启动帧所允许的、经扩展的测量时钟偏差范围，有效值为0~1875μT */
        1,        //uint16_t pDelayCompensationA;           /*!< 用来补偿 A 通道接收延迟的值。它覆盖了高达 2.5μs 的假定传播延迟，有效值为 0~200μT
        1,        //uint16_t pDelayCompensationB;           /*!< 用来补偿 B 通道接收延迟的值。它覆盖了高达 2.5μs 的假定传播延迟，有效值为 0~200μT
        //这个keyslot收发的时候，这个参数不能一致
        7,        //uint16_t pKeySlotId;                    /*!< 用于发送启动帧、同步帧或指定单时隙帧的时隙标识符（ID），有效值为 1~1023时隙。 */
        1,        //uint8_t pKeySlotUsedForStartup;         /*!< 指 示 关 键 时 隙 是 否 用 于 发 送 启 动 帧 的 标 志 ， 有 效 值 为 0 和 1 ， 若pKeySlotUsedForStartup 设置为 1，则 */
        1,        //uint8_t pKeySlotUsedForSync;            /*!< 指 示 关 键 时 隙 是 否 用 于 发 送 同 步 帧 的 标 志 ， 有 效 值 为 0 和 1 ， 若pKeySlotUsedForStartup 设置为 1，则 pKeySlotUsedForSync */
        21,       //uint16_t pLatestTx;                     /*!< 在动态段最后可以开始帧发送的微时隙数，有效值为 0~7980 */
        23,       //uint8_t pMicroInitialOffsetA;           /*!< 由 pMacroInitialOffset[CH]描述的最接近的宏节拍边界和第二时间基准点之间的微节拍数，有效值为 0~239μT。
        23,       //uint8_t pMicroInitialOffsetB;           /*!< 由 pMacroInitialOffset[CH]描述的最接近的宏节拍边界和第二时间基准点之间的微节拍数，有效值为 0~239μT
        200000,   //uint32_t pMicroPerCycle;                /*!< 一个通信周期的总微节拍数量。需要注意的是 pMicroPerCycle= gMacroPerCycle* pMicroPerMacroNom，有效值为 640~640000μT */
        601,      //uint32_t pdMaxDrift;                    /*!< 两个节点以非同步时钟运行一个通信周期以上时，两者之间的最大漂移偏差，有效值为 2~1923μT
        40,       //uint8_t pMicroPerMacroNom;              /*!< 一个宏节拍包含的微节拍数量，有效值为 40~240μT
        601,      //uint16_t pRateCorrectionOut;            /*!< 允许最大速率修正值的大小，有效值为 2~1923μT。 */
        141,      //uint16_t pOffsetCorrectionOut;          /*!< 允许最大偏差修正值的大小，有效值为 2~1923μT */
        0,        //uint8_t pSingleSlotEnabled;             //!< 启动后节点是否将进入单时隙模式标志位，有效值为 0 和 1
        //FR_CHANNEL_A
        2,        //uint8_t pWakeupChannel;                 //!< 节点用于发送唤醒模式的通道，0 为 A 通道，1 为 B 通道
        33,       //uint8_t pWakeupPattern;                 /*!< 当节点进入唤醒发送状态时，形成唤醒模式的唤醒符号重复次数，有效值为 2~63次。 */
        56,       //uint16_t pDecodingCorrection;           /*!< 接收器用于计算主时间基准点和次级时间基准点之间差异的值，有效值为14~143μT。
        0,        //uint16_t keySlotHeaderCrc;              /*!< 关键帧头 CRC 校验和，应用本模块时填写 0 即可，实际校验和由设备软件自动计算*/
        0,        //uint8_t pExternOffsetCorrection;        /*!< 进行主机请求的外部偏差修正时，NIT 加上或减去的微节拍数，有效值为 0~7μT */
        0         //uint8_t pExternRateCorrection;          /*!< 进行主机请求的外部速率修正时，周期加上或减去的微节拍数，有效值为0~7μT */


};

//初始化flexray总线的控制器参数
FlexrayCCParameter_t mFlexrayCCParameter{

        0,   // uint8_t freezeMode;                /*!< 当总线失败时是否进入冻结模式，有效值为 1（使能）或 0（关闭） */
        1,   // uint8_t channelA;                  /*!< 是否使用通道 A，1（使用）或 0（不使用） */
        1,   // uint8_t channelB;                  /*!< 当通道 A 启用时，端口 B 连接到通道 B（双通道模式）；当通道 A 禁用时，端口 A 连接到通道 B（单通道模式）。是否使用通道 B，1（使用）或 0（不使用） */
        1,   // uint8_t syncFrameFilter;           /*!< 是否启动同步帧过滤，1（使能）或 0（禁用） */
        0    // uint8_t bitRate;     /*!< 总线通讯速率选择，0（10Mbps），1（5Mbps），2（2.5Mbps），3（8Mbps） */
};
//这个结构体自回环测试时，发送和接受的时隙号，第二个参数，需要一致
FlexrayMsgBufferParameter_t gFlexrayMsgBufferParameter[20]=
{

    // FR_DF_1   接收
    {MBID_DEFINE_BY_DEVICE, 55,       0,   20,           0,    1,  1,       0,       16,              0 },
    // FR_DF_2   接收
    {MBID_DEFINE_BY_DEVICE, 55,       0,   20,           0,    1,  1,       2,       16,              0 },
    //FR_MP_5    发送
    {MBID_DEFINE_BY_DEVICE, 42,       0,   11,           1,    1,  1,       10,       16,              1  }

};


//构造
FlexRayThread::FlexRayThread()  {
    m_sendData_MasterControl = {0, 0, 0, 0, 0};
    m_sendData_Drive        = {0, 0, 0, 0, 0, 0};
    m_sendData_MP5          = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    m_instance = 0;
    m_instance2 = 0;



}
bool FlexRayThread::checkoutflexray()
{
    const char* charPtr = "192.168.0.7";  // 使用 const char* 类型
    const char* charPtr2 = "192.168.0.8";  // 使用 const char* 类型
    // 使用短期线程带超时打开设备
    auto openWithTimeout = [&](const char* ip, sint8 &outInst, int timeoutMs)->bool {
        QFuture<sint8> fut = QtConcurrent::run([ip]() { return UFr_Open_Wrapper(const_cast<char*>(ip)); });
        QFutureWatcher<sint8> watcher;
        QEventLoop loop;
        QObject::connect(&watcher, &QFutureWatcher<sint8>::finished, &loop, &QEventLoop::quit);
        watcher.setFuture(fut);
        QTimer timer;
        timer.setSingleShot(true);
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(timeoutMs);
        loop.exec();
        if (!fut.isFinished()) return false;
        outInst = fut.result();
        return outInst > 0;
    };

    if (!openWithTimeout(charPtr, m_instance, 10000)) {
        return 0;
    }
    if (!openWithTimeout(charPtr2, m_instance2, 10000)) {
        return 0;
    }

    Init_instance();
    int continueLoop = 1;  // 标志变量，控制是否继续循环
    uint8_t frnodeState,curCycle;

    UFr_GetFrNodeState_Wrapper(m_instance, 0, frnodeState, curCycle);
    switch (frnodeState) {
        case FR_POCSTATE_CONFIG:
            continueLoop = 0;
            break;
        case FR_POCSTATE_DEFAULT_CONFIG:
            continueLoop = 0;
            break;
        case FR_POCSTATE_HALT:
            continueLoop = 0;
            break;
        case FR_POCSTATE_NORMAL_ACTIVE:
            continueLoop = 0;
            break;
        case FR_POCSTATE_NORMAL_PASSIVE:
            continueLoop = 0;
            break;
        case FR_POCSTATE_READY:
            continueLoop = 0;
            break;
        case FR_POCSTATE_STARTUP:
            continueLoop = 0;
            break;
        case FR_POCSTATE_WAKEUP:
            continueLoop = 0;
            break;
        default:
            continueLoop = 0;
            break;
    }

    if(continueLoop == 0)
    {
        return 1;
    }
    return 0;

}
void FlexRayThread::updatestate()
{
    if (!m_running) return;

    uint8_t frnodeState1, curCycle1;
    uint8_t frnodeState2, curCycle2;

    UFr_GetFrNodeState_Wrapper(m_instance, 0, frnodeState1, curCycle1);
    UFr_GetFrNodeState_Wrapper(m_instance2, 0, frnodeState2, curCycle2);

    // A通道 (实例1, 192.168.0.7) → NORMAL_ACTIVE(0x05) 或 NORMAL_PASSIVE(0x06)=1
    bool chAOk = (frnodeState1 == FR_POCSTATE_NORMAL_ACTIVE ||
                  frnodeState1 == FR_POCSTATE_NORMAL_PASSIVE);
    // B通道 (实例2, 192.168.0.8)
    bool chBOk = (frnodeState2 == FR_POCSTATE_NORMAL_ACTIVE ||
                  frnodeState2 == FR_POCSTATE_NORMAL_PASSIVE);

    emit abChannelChanged(chAOk ? 1 : 0, chBOk ? 1 : 0);
}
FlexRayThread::~FlexRayThread() {
    // 请求停止，不再调用 wait()（不是 QThread）
    stop();
}

//发送
void FlexRayThread::flexRayDataSend(QVector<uint16> masterData, QVector<uint16> driveData)
{
    m_sendData_MasterControl = masterData;
    m_sendData_Drive        = driveData;
    LOG(QString("FlexRay TX master[0]=0x%1 drive[0]=0x%2").arg(masterData.value(0),4,16,QLatin1Char('0')).arg(driveData.value(0),4,16,QLatin1Char('0')));
}

// FR_MP_5 发送 (槽ID=42, cycleOffset=11)
void FlexRayThread::flexRayDataSend_MP5(QVector<uint16> sendData)
{
    m_sendData_MP5 = sendData;
    LOG(QString("FlexRay TX MP5[0]=0x%1 len=%2").arg(sendData.value(0),4,16,QLatin1Char('0')).arg(sendData.size()));
}

//发送配置
void FlexRayThread::Init_instance()
{
    uint8 ret;
    //配置 cluster参数
    if(UFr_Set_Cluster_Parameter_Wrapper(m_instance,0,&mFlexrayClusterParameter) != CMD_SUCCESS)
    {
        emit error("Set_Cluster_Parameter fail!");
        this->stop();
        return;
    }

    //配置 节点参数
    if(UFr_Set_Node_Parameter_Wrapper(m_instance,0,&mFlexrayNodeParameter) != CMD_SUCCESS)
    {
        emit error("UFr_Set_Node_Parameter fail!");
        this->stop();
        return;
    }

    //配置 CC参数
    if(UFr_Set_CC_Parameter_Wrapper(m_instance,0,&mFlexrayCCParameter) != CMD_SUCCESS)
    {
        emit error("UFr_Set_CC_Parameter fail!");
        this->stop();
        return;
    }

    //配置 时隙参数
    ret = UFr_Set_Slot_Parameter_Wrapper(m_instance,0,&gFlexrayMsgBufferParameter[0]);
    if(ret != CMD_SUCCESS)
    {
        emit error("UFr_Set_Slot_Parameter FAIL!");
        this->stop();
        return;
    }

    //配置 时隙参数
    ret = UFr_Set_Slot_Parameter_Wrapper(m_instance,0,&gFlexrayMsgBufferParameter[1]);
    if(ret != CMD_SUCCESS)
    {
        emit error("UFr_Set_Slot_Parameter FAIL!");
        this->stop();
        return;
    }

    //配置 发送时隙 FR_MP_5
    ret = UFr_Set_Slot_Parameter_Wrapper(m_instance,0,&gFlexrayMsgBufferParameter[2]);
    if(ret != CMD_SUCCESS)
    {
        emit error("UFr_Set_Slot_Parameter MP5 FAIL!");
        this->stop();
        return;
    }


    //配置 使能
    if(UFr_Configure_Node_Wrapper(m_instance,0) != CMD_SUCCESS)
    {
        emit error("UFr_Configure_Node fail!");
        this->stop();
        return ;
    }

    //启动 节点，进入总线建立连接状态
    if((ret = UFr_Start_Node_Wrapper(m_instance,0)) != CMD_SUCCESS)
    {
        emit error(QString("UFr_Start_Node fail[%1]!").arg(ret));
        this->stop();
        return ;
    }
    //启动 接收数据上传操作
    if(UFr_StartFrRxUpload_Wrapper(m_instance,0) != CMD_SUCCESS)
    {
        emit error("UFr_StartFrRxUpload fail!");
        this->stop();
        return ;
    }



    //配置 cluster参数
    if(UFr_Set_Cluster_Parameter_Wrapper(m_instance2,0,&mFlexrayClusterParameter) != CMD_SUCCESS)
    {
        emit error("Set_Cluster_Parameter fail!");
        this->stop();
        return;
    }

    //配置 节点参数
    if(UFr_Set_Node_Parameter_Wrapper(m_instance2,0,&mFlexrayNodeParameter_2) != CMD_SUCCESS)
    {
        emit error("UFr_Set_Node_Parameter fail!");
        this->stop();
        return;
    }

    //配置 CC参数
    if(UFr_Set_CC_Parameter_Wrapper(m_instance2,0,&mFlexrayCCParameter) != CMD_SUCCESS)
    {
        emit error("UFr_Set_CC_Parameter fail!");
        this->stop();
        return;
    }

    //配置 时隙参数
    ret = UFr_Set_Slot_Parameter_Wrapper(m_instance2,0,&gFlexrayMsgBufferParameter[0]);
    if(ret != CMD_SUCCESS)
    {
        emit error("UFr_Set_Slot_Parameter FAIL!");
        this->stop();
        return;
    }

    //配置 时隙参数
    ret = UFr_Set_Slot_Parameter_Wrapper(m_instance2,0,&gFlexrayMsgBufferParameter[1]);
    if(ret != CMD_SUCCESS)
    {
        emit error("UFr_Set_Slot_Parameter FAIL!");
        this->stop();
        return;
    }

    //配置 发送时隙 FR_MP_5
    ret = UFr_Set_Slot_Parameter_Wrapper(m_instance2,0,&gFlexrayMsgBufferParameter[2]);
    if(ret != CMD_SUCCESS)
    {
        emit error("UFr_Set_Slot_Parameter MP5 FAIL!");
        this->stop();
        return;
    }


    //配置 使能
    if(UFr_Configure_Node_Wrapper(m_instance2,0) != CMD_SUCCESS)
    {
        emit error("UFr_Configure_Node fail!");
        this->stop();
        return ;
    }

    //启动 节点，进入总线建立连接状态
    if((ret = UFr_Start_Node_Wrapper(m_instance2,0)) != CMD_SUCCESS)
    {
        emit error(QString("UFr_Start_Node fail[%1]!").arg(ret));
        this->stop();
        return ;
    }

    //启动 接收数据上传操作
    if(UFr_StartFrRxUpload_Wrapper(m_instance2,0) != CMD_SUCCESS)
    {
        emit error("UFr_StartFrRxUpload fail!");
        this->stop();
        return ;
    }

}
//开始
void FlexRayThread::startFlexRayOperation() {
    LOG("startFlexRayOperation 进入");
    LOG(QString("startFlexRayOperation: currentThread=%1 this->thread=%2").arg((quintptr)QThread::currentThread(),0,16).arg((quintptr)this->thread(),0,16));
    // QMutexLocker locker(&m_mutex);
    if (!m_running) {
        LOG("m_running=false, 开始初始化");
        if (m_instance > 0) { LOG("关闭旧实例1"); UFr_Stop_Wrapper(m_instance, 0); UFr_Close_Wrapper(m_instance); }
        if (m_instance2 > 0) { LOG("关闭旧实例2"); UFr_Stop_Wrapper(m_instance2, 0); UFr_Close_Wrapper(m_instance2); }

        LOG("打开设备 192.168.0.7");
        const char* charPtr = "192.168.0.7";
        const char* charPtr2 = "192.168.0.8";

        // 将 Open 操作放到短期线程中执行并带有超时保护（10s）
        auto openWithTimeout = [&](const char* ip, sint8 &outInst, int timeoutMs)->bool {
            QFuture<sint8> fut = QtConcurrent::run([ip]() {
                return UFr_Open_Wrapper(const_cast<char*>(ip));
            });
            QFutureWatcher<sint8> watcher;
            QEventLoop loop;
            QObject::connect(&watcher, &QFutureWatcher<sint8>::finished, &loop, &QEventLoop::quit);
            watcher.setFuture(fut);
            QTimer timer;
            timer.setSingleShot(true);
            QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
            timer.start(timeoutMs);
            loop.exec();
            if (!fut.isFinished()) {
                LOG(QString("UFr_Open_Wrapper(%1) 超时 %2 ms").arg(ip).arg(timeoutMs));
                return false;
            }
            outInst = fut.result();
            return outInst > 0;
        };

        if (!openWithTimeout(charPtr, m_instance, 10000)) {
            LOG("UFr_Open_Wrapper 192.168.0.7 失败或超时!");
            emit error("总线启动失败");
            return;
        }
        LOG(QString("m_instance=%1").arg(m_instance));

        LOG("打开设备 192.168.0.8");
        if (!openWithTimeout(charPtr2, m_instance2, 10000)) {
            LOG("UFr_Open_Wrapper 192.168.0.8 失败或超时!");
            emit error("总线启动失败");
            return;
        }
        LOG(QString("m_instance2=%1").arg(m_instance2));

        LOG("调用 Init_instance...");
        Init_instance();
        LOG("Init_instance 完成");
        m_running = true;
        int continueLoop = 1;
        int timeoutCount = 0;
        uint8_t frnodeState,curCycle,frnodeState2,curCycle2;
        LOG("开始等待节点就绪...");
        while(continueLoop && timeoutCount < 100)
        {
            UFr_GetFrNodeState_Wrapper(m_instance, 0, frnodeState, curCycle);
            UFr_GetFrNodeState_Wrapper(m_instance2, 0, frnodeState2, curCycle2);

            if (timeoutCount % 10 == 0) {
                LOG(QString("FlexRay state: node1=0x%1 cycle=%2 node2=0x%3 cycle2=%4").arg(frnodeState,0,16).arg(curCycle).arg(frnodeState2,0,16).arg(curCycle2));
            }

            bool node1Ready = (frnodeState == FR_POCSTATE_NORMAL_ACTIVE || frnodeState == FR_POCSTATE_NORMAL_PASSIVE);
            bool node2Ready = (frnodeState2 == FR_POCSTATE_NORMAL_ACTIVE || frnodeState2 == FR_POCSTATE_NORMAL_PASSIVE);
            if(node1Ready && node2Ready)
            {
                LOG(QString("FlexRay 节点就绪! node1=0x%1 node2=0x%2").arg(frnodeState,0,16).arg(frnodeState2,0,16));
                continueLoop = 0;
                break;
            }
            QThread::msleep(100);
            timeoutCount++;
        }

        if (timeoutCount >= 100) {
            LOG("节点启动超时(10s)!");
            emit error("FlexRay节点启动超时(10s)");
            m_running = false;
            UFr_Close_Wrapper(m_instance);
            UFr_Close_Wrapper(m_instance2);
            return;
        }

            LOG("排入 runLoop 开始执行...");
            // 在 worker 所在线程的事件循环中排入 runLoop 执行（非阻塞）
            QMetaObject::invokeMethod(this, "runLoop", Qt::QueuedConnection);
            LOG("startFlexRayOperation 完成");
    } else {
        LOG("m_running 已为true，跳过初始化");
    }

}


//停止
void FlexRayThread::stop() {
    // 首先标记停止请求
    m_running = false;

    // 立即尝试调用 UFr_Stop_Wrapper（不加锁）以唤醒可能在 UFr_Receive_Wrapper 中阻塞的调用。
    if (m_instance > 0) {
        LOG("stop(): 调用 UFr_Stop_Wrapper(m_instance, 0) 以唤醒阻塞");
        UFr_Stop_Wrapper(m_instance, 0);
    }
    if (m_instance2 > 0) {
        LOG("stop(): 调用 UFr_Stop_Wrapper(m_instance2, 0) 以唤醒阻塞");
        UFr_Stop_Wrapper(m_instance2, 0);
    }

    // 作为后备，尝试关闭实例以释放底层资源（最多只能作为最后手段）
    if (m_instance > 0) {
        LOG("stop(): 尝试调用 UFr_Close_Wrapper(m_instance) 作为后备");
        UFr_Close_Wrapper(m_instance);
    }
    if (m_instance2 > 0) {
        LOG("stop(): 尝试调用 UFr_Close_Wrapper(m_instance2) 作为后备");
        UFr_Close_Wrapper(m_instance2);
    }

    // 不在 stop() 中阻塞等待退出；调用者可以从 UI 线程轮询 isRunning() 或者监听信号
}

void FlexRayThread::runLoop()
{
    LOG("FlexRay runLoop 启动");
    QElapsedTimer stateTimer;
    stateTimer.start();
    int noDataCount = 0;  // 连续无帧计数
    static int loopCnt = 0;

    while (m_running)
    {
        // 每1秒检查节点状态
        if (stateTimer.elapsed() >= 1000) {
            updatestate();
            stateTimer.restart();
        }

        // 读取本地副本的实例句柄，尽量缩小锁的作用域
        sint8 localInstance = 0;
        {
            QMutexLocker locker(&m_mutex);
            localInstance = m_instance;
        }

        uint ret = CMD_FAIL_STATUS;
        uint16 rcvData[20];
        QVector<uint16> t;
        uFlexrayFrameHead_t uFlexrayFrameHead;

        loopCnt++;
        bool gotData = false;

        if (localInstance > 0) {
            // 调用底层阻塞接收（有超时参数），但不持有 m_mutex
            ret = UFr_Receive_Wrapper(localInstance, &uFlexrayFrameHead, rcvData, 1000);
        } else {
            // 如果没有有效实例，短等待并跳过
            QThread::msleep(200);
            continue;
        }

        if (ret == CMD_SUCCESS)
        {
            gotData = true;
            for (int i = 0; i < uFlexrayFrameHead.payloadLength; i++) {
                t.append(rcvData[i]);
            }
            int cyclecount = uFlexrayFrameHead.cycleCount;
            if (loopCnt <= 3)
                LOG(QString("FlexRay 收到 cycle=%1 payload=%2").arg(cyclecount).arg(uFlexrayFrameHead.payloadLength));
            if ((cyclecount % 16) == 2) {
                emit flexRayDataReceived(t, 2);
            } else {
                emit flexRayDataReceived(t, 1);
            }

            // AB通道 → ch4 (仅收到帧时才更新)
            t.clear();
            t.push_back(static_cast<uint16_t>(uFlexrayFrameHead.Channel));
            t.push_back(uFlexrayFrameHead.frameID);
            emit flexRayDataReceived(t, 4);
        }

        t.clear();

        if (!gotData) {
            noDataCount++;
            if (noDataCount >= 3) {  // 3轮无数据≈3秒, 强制灭AB
                emit abFrameTimeout();
                noDataCount = 0;
            }
            if (loopCnt <= 3 || loopCnt % 10 == 0)
                LOG(QString("FlexRay runLoop 第%1轮 无数据").arg(loopCnt));
            QThread::msleep(500);
        }
        else
        {
            noDataCount = 0;  // 有数据就重置
            if (loopCnt <= 3)
                LOG(QString("FlexRay runLoop 第%1轮 有数据").arg(loopCnt));
        }

        // FR_MP_5 发送 (槽ID=42, cycleOffset=11, 方向=TX)
        {
            // 获取发送缓冲的本地副本，避免直接在发送过程中持锁
            QVector<uint16> sendMP5Copy;
            {
                QMutexLocker locker(&m_mutex);
                sendMP5Copy = m_sendData_MP5;
            }

            uFlexrayFrame_t mFlexrayFrame_MP5;
            int payloadLen = sendMP5Copy.size();
            mFlexrayFrame_MP5.payloadLength = payloadLen;
            for (int i = 0; i < payloadLen; i++) {
                mFlexrayFrame_MP5.data[i] = sendMP5Copy.at(i);
            }

            int tret = UFr_Transmit_Wrapper(localInstance, 0,
                                           gFlexrayMsgBufferParameter[2].msgBufferID,
                                           mFlexrayFrame_MP5.data,
                                           mFlexrayFrame_MP5.payloadLength);
            if (tret != CMD_SUCCESS && loopCnt <= 3) {
                LOG(QString("FlexRay TX MP5 fail ret=%1").arg(tret));
            }
        }

        static int clearCount = 0;
        if (++clearCount >= 100) {
            UFr_ClearRcvBuffer_Wrapper(localInstance, 0);
            clearCount = 0;
        }

        QThread::msleep(1);
    }

    LOG("FlexRay runLoop 退出");
}
