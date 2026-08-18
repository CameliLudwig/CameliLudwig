#ifndef UBUSCOMMLIBWRAPPER_H
#define UBUSCOMMLIBWRAPPER_H


#define DLLIMPORT __declspec (dllexport)
#define MBID_DEFINE_BY_DEVICE    0xff
#define ReplayframebufferMaxbyteslength   1340


typedef unsigned char       boolean;


typedef signed char         sint8;
typedef unsigned char       uint8_t;
typedef unsigned char       uint8;
typedef signed short int    sint16;
typedef unsigned short int  uint16;
typedef unsigned short int  uint16_t;
typedef signed long int     sint32;
typedef unsigned long int   uint32;
typedef long long           sint64;
typedef unsigned long long  uint64;
/* Message buffer ID type */
typedef uint8_t MessageBufferID_t;

/*
 * Flexray global cluster parameters structure
 */
typedef struct fcpt
{
    uint8_t gColdstartAttempts;             /*!< Number of coldstart node in network (Range: 2->31) */
    uint8_t gdActionPointOffset;            /*!< Number of macroticks the action point is offset from the beginning of a static slot
                                                 or symbol window; value into range: 1-63MT. */
    uint8_t gdCASRxLowMax;                  /*!< Upper limit of the CAS acceptance window; value into range: 67-99 gdBit */
    uint8_t gdDynamicSlotIdlePhase;         /*!< Duration of the idle phase within a dynamic slot. from 0 to 2 */
    uint8_t gdMinislot;
    uint8_t gdMinislotActionPointOffset;    /*!< Number of macroticks the minislot action point is offset from the beginning of a minislot
                                                value into range: 1-31MT */
    uint16_t gdStaticSlot;                  /*!< Duration of a static slot (4->661 MT) */
    uint8_t gdSymbolWindow;
    uint8_t gdTSSTransmitter;               /*!< number of bits in the Transmission Start Sequence(3-15gdBit) */
    uint8_t gdWakeupSymbolRxIdle;           /*!< Number of bits used by the node to test the duration of
                                                 the 'idle' portion of a received wakeup symbol (14-59gdBit) */
    uint8_t gdWakeupSymbolRxLow;            /*!< Number of bits used by the node to test
                                                 the LOW portion of a received wakeup symbol (11-59gdBit) */
    uint16_t gdWakeupSymbolRxWindow;        /*!< The size of the window used to detect wakeup (76-301 gdBit) */
    uint8_t gdWakeupSymbolTxIdle;           /*!< Number of bits used by the node to transmit the 'idle' part of a wakeup symbol (45-180 gdBit) */
    uint8_t gdWakeupSymbolTxLow;              /*!< Number of bits used by the node to transmit the LOW part of a wakeup symbol (15-60 gdBit) */
    uint32  gListenNoise;
    uint8_t gNetworkManagementVectorLength; /*!< The length of the NMVector in byte */
    uint16_t gMacroPerCycle;                /*!< Number of macroticks in a communication cycle (10 - 16000MT) */
    uint8_t gMaxWithoutClockCorrectionFatal;
    uint8_t gMaxWithoutClockCorrectionPassive;
    uint16_t gNumberOfMinislots;
    uint16_t gNumberOfStaticSlots;
    uint16_t gOffsetCorrectionStart;        /*!< Start of the offset correction phase within the NIT,
                                                 expressed as the number of macroticks from the start of cycle (9 - 15999 MT)
                                                 Note that this value should greater than
                                                 gdStaticSlot* gNumberOfStaticSlots + gdMinislot*gNumberOfMinislots + gdSymbolWindow */
    uint8_t gPayloadLengthStatic;           /*!< Payload length of a static frame */
    uint8_t gSyncNodeMax;                   /*!< Maximum number of nodes that may send frames with the sync frame indicator bit set to one. */

} FlexrayClusterParameter_t;

/*
 * Flexray node parameters structure
 */
typedef struct fnpt
{
    uint32  pdListenTimeout;               /*!< Value for the startup listen timeout and wakeup listen timeout */
    uint8_t pMacroInitialOffsetA;           /*!< 2-68MT */
    uint8_t pMacroInitialOffsetB;           /*!< 2-68MT */
    uint8_t pPayloadLengthDynMax;           /*!< Maximum payload length for dynamic frames */
    uint8_t pAllowHaltDueToClock;			/*!< 1(true) or 0(false) */
    uint8_t pAllowPassiveToActive;          /*!< If set to zero, the CC is not allowed to transition from normal passive to normal active */
    uint8_t pClusterDriftDamping;            /*!< Local cluster drift damping factor used for rate correction (0-20uT) */
    uint16_t pdAcceptedStartupRange;
    uint16_t pDelayCompensationA;
    uint16_t pDelayCompensationB;
    uint16_t pKeySlotId;                    /*!< ID of the slot used to transmit the startup frame, sync frame, or designated single slot frame */
    uint8_t pKeySlotUsedForStartup;         /*!< key slot is startup frame or not ?
                                                 1(true) or 0(false) */
    uint8_t pKeySlotUsedForSync;            /*!< key slot is sync frame or not ?
                                                 1(true) or 0(false) */
    uint16_t pLatestTx;                     /*!< Number of the last minislot in which a frame transmission can start in the dynamic segment */
    uint8_t pMicroInitialOffsetA;
    uint8_t pMicroInitialOffsetB;
    uint32  pMicroPerCycle;                /*!< Nominal number of microticks in the communication cycle of the local node (640-640000 uT) */
    uint32  pdMaxDrift;
    uint8_t pMicroPerMacroNom;
    uint16_t pRateCorrectionOut;            /*!< Magnitude of the maximum permissible rate correction value (2-1923 uT) */
    uint16_t pOffsetCorrectionOut;          /*!< Magnitude of the maximum permissible offset correction value (13-15567 uT) */
    uint8_t pSingleSlotEnabled;             /*!< Flag indicating whether or not the node shall enter single slot mode following startup
                                                 1(true) or 0(false) */
    uint8_t pWakeupChannel;                 /*!< Channel used by the node to send a wakeup pattern: false -> A, true -> B
                                                 1(true) or 0(false) */
    uint8_t pWakeupPattern;                 /*!< Number of repetitions of the wakeup symbol that are combined to form a wakeup pattern when the node
                                                 enters the POC:wakeup send state */
    uint16_t pDecodingCorrection;
    uint16_t keySlotHeaderCrc;             /*!< header CRC for key slot (0x00-0x7FF) */
    uint8_t pExternOffsetCorrection;        /*!< Number of microticks added or subtracted to the NIT to carry out a host-requested external offset correction (0-7uT) */
    uint8_t pExternRateCorrection;          /*!< Number of microticks added or subtracted to the cycle to carry out a host-requested external rate correction (0-7uT) */

} FlexrayNodeParameter_t;

/*
 * bitrate defination
 */
typedef enum fbrt
{
    UFR_BITRATE_10M  = 0U,       /*!< 10 Mbit/s */
    UFR_BITRATE_5M   = 1U,       /*!< 5 Mbit/s */
    UFR_BITRATE_2_5M = 2U,       /*!< 2.5 Mbit/s */
    UFR_BITRATE_8M   = 3U        /*!< 8.0 Mbit/s */
} FlexrayBitrate_t;

typedef enum
{
    FR_WORKDMODE_NORMAL = 0,
    FR_WORKDMODE_PLAYBACK
} Fr_WORKMODE_type;

typedef enum
{
    FR_RECEIVE_NULLFRAME = 0,  //1:receive nullframe, 0:disable receive nullframe, default is 0
    FR_AUTORESTART_ONHALT,     //1:enable auto restart on Node POC entering halt state
                               //0:disable auto restart on Node POC entering halt state
                               //default is 1
} Fr_PARAMETER;

/*
 * Flexray Communication Controller parameters structure
 */
typedef struct fccpt
{
    uint8_t freezeMode;                /*!< Transition to freeze mode or not(1/0) when system bus fail happen */
                                    /* 1(true) or 0(false) */
    uint8_t channelA;                  /*!< True: use channel A/False: not use channel A */
                                    /* 1(true) or 0(false) */
    uint8_t channelB;                  /*!< True: If channel A is enabled, port B is connected  to channel B (dual channel mode)
                                               If channel A is disabled, port A is connected to channel B(single channel mode)
                                         False: not use channel B */
                                    /* 1(true) or 0(false) */
    uint8_t syncFrameFilter;		/* 1(true) or 0(false) */
    uint8_t bitRate;

} FlexrayCCParameter_t;

/*
 * Flexray slot parameters structure
 */
typedef struct fspt
{
    MessageBufferID_t msgBufferID;  /*!< Message buffer ID, Value with all 1s means that it is allocated by driver */

    uint16_t frameID;           /*!< Slot ID in which frame is transmitted/received */


    uint8_t PPI;                   /*!< Payload Preamble Indicator */
                                /* 1(true) or 0(false) */
    uint8_t payloadLength;      /*!< Length in 2-byte unit, only for dynamic slot transmission */

    uint8_t isTx;                  /*!< True: Tx buffer, false: Rx buffer */
                                /* 1(true) or 0(false) */
    uint8_t channelA;              /*!< True: enable transfer on channel A, false otherwise */
                                /* 1(true) or 0(false) */
    uint8_t channelB;              /*!< True: enable transfer on channel B, false otherwise.
                                    Note that different combinations of channelA and channelB fields shall have
                                    different effects. Please see Channel Assignment Description table in RM for details */
                                /* 1(true) or 0(false) */

    uint8_t baseCycle;      /* 0~63, It must be less than the repetition cycle */
    uint8_t repetitionCycle; /* =2^N, that is N-th power of 2, where N is a number between 0 and 6 */


    uint8_t repeatTx;              /*!< Only used if this is a tx individual buffer.
                                    True: buffer is transmited again automatically.
                                    False: buffer is trasmitted only once. */
                                /* 1(true) or 0(false) */

} FlexrayMsgBufferParameter_t;

/*
 * FlexRay FIFO parameters structure
 */
typedef struct
{
    uint16_t messageIdAccVal;                 /* 16 bits *//*!< MESSAGE ID Acceptance Filter Value */
    uint16_t messageIdAccMask;                /* 16 bits *//*!< MESSAGE ID Acceptance Filter Mask. This is ANDed with MESSAGE ID and compare with fidAccVal */
    uint16_t frameIdRejVal;                   /* 11 bits *//*!< FRAME ID rejection Filter Mask */
    uint16_t frameIdRejMask;                  /* 11 bits *//*!< FRAME ID rejection Filter Mask. This is ANDed with FRAME ID and compare with fidRejVal */
    uint8_t rangeFilter1Enable;                   /*!< Range filter enable. 1(True): Enable. 0(False): Disable */
    uint16_t rangeFilter1Upper;                   /* 11 bits *//*!< Frame ID upper value for range filter */
    uint16_t rangeFilter1Lower;                   /* 11 bits *//*!< Frame ID lower value for range filter */
    uint8_t rangeFilter1Mode;                     /*!< Range filter mode. 0:Accept mode. 1:Reject mode */
    uint8_t rangeFilter2Enable;                   /*!< Range filter enable. 1(True): Enable. 0(False): Disable */
    uint16_t rangeFilter2Upper;                   /* 11 bits *//*!< Frame ID upper value for range filter */
    uint16_t rangeFilter2Lower;                   /* 11 bits *//*!< Frame ID lower value for range filter */
    uint8_t rangeFilter2Mode;                     /*!< Range filter mode. 0:Accept mode. 1:Reject mode */
    uint8_t rangeFilter3Enable;                   /*!< Range filter enable. 1(True): Enable. 0(False): Disable */
    uint16_t rangeFilter3Upper;                   /* 11 bits *//*!< Frame ID upper value for range filter */
    uint16_t rangeFilter3Lower;                   /* 11 bits *//*!< Frame ID lower value for range filter */
    uint8_t rangeFilter3Mode;                     /*!< Range filter mode. 0:Accept mode. 1:Reject mode */
    uint8_t rangeFilter4Enable;                   /*!< Range filter enable. 1(True): Enable. 0(False): Disable */
    uint16_t rangeFilter4Upper;                   /* 11 bits *//*!< Frame ID upper value for range filter */
    uint16_t rangeFilter4Lower;                   /* 11 bits *//*!< Frame ID lower value for range filter */
    uint8_t rangeFilter4Mode;                     /*!< Range filter mode. 0:Accept mode. 1:Reject mode */
} FlexrayFifoParameterFlat_t;


typedef struct flexrayframet
{
	uint32 serial_number;
	uint32 timestamp_s;
	uint32 timestamp_us;
	uint8_t payloadPreambleIndicator;
	uint8_t nullFrameIndicator;			/* 1(true) or 0(false) */
	uint8_t syncFrameIndicator;			/* 1(true) or 0(false) */
	uint8_t startupFrameIndicator;		/* 1(true) or 0(false) */
	uint16_t frameID;
	uint8_t  Channel;
	uint8_t cycleCount;
	uint8_t payloadLength;
	uint16_t headerCrc;
	uint16_t slotStatus;
	uint16 data[127];

} uFlexrayFrame_t;

typedef struct flexrayframethead
{
	uint32 serial_number;
	uint32 timestamp_s;
	uint32 timestamp_us;
	uint8_t payloadPreambleIndicator;
	uint8_t nullFrameIndicator;			/* 1(true) or 0(false) */
	uint8_t syncFrameIndicator;			/* 1(true) or 0(false) */
	uint8_t startupFrameIndicator;		/* 1(true) or 0(false) */
	uint16_t frameID;
	uint8_t  Channel;
	uint8_t cycleCount;
	uint8_t payloadLength;
	uint16_t headerCrc;
	uint16_t slotStatus;

} uFlexrayFrameHead_t;

/*
 *
 */
typedef struct
{
    uint8_t rangeFilterEnable;                   /*!< Range filter enable. 1(True): Enable. 0(False): Disable */
    uint16_t rangeFilterUpper;                   /* 11 bits *//*!< Frame ID upper value for range filter */
    uint16_t rangeFilterLower;                   /* 11 bits *//*!< Frame ID lower value for range filter */
    uint8_t rangeFilterMode;                     /*!< Range filter mode. 0:Accept mode. 1:Reject mode */
} FlexrayRangeFilter_t;

/*
 * FlexRay FIFO parameters structure
 */
typedef struct
{
    uint8_t reserve;                           /* reserve byte,user don't need set */
    uint16_t messageIdAccVal;                 /* 16 bits *//*!< MESSAGE ID Acceptance Filter Value */
    uint16_t messageIdAccMask;                /* 16 bits *//*!< MESSAGE ID Acceptance Filter Mask. This is ANDed with MESSAGE ID and compare with fidAccVal */
    uint16_t frameIdRejVal;                   /* 11 bits *//*!< FRAME ID rejection Filter Mask */
    uint16_t frameIdRejMask;                  /* 11 bits *//*!< FRAME ID rejection Filter Mask. This is ANDed with FRAME ID and compare with fidRejVal */
    FlexrayRangeFilter_t rangeFilter[4];      /*!< Frame ID range Filter. Maximum number of range filter is 4 */
    uint8_t numberOfRangeFilter;
} FlerayFifoParameter_t;



/*
 * FlexRay replay frame header structure
 */
typedef struct ffrplht
{
    uint16_t frameID;
    uint32   cycleOffset;
    uint8_t cycleCount;
    uint8_t PPI;
    uint8_t payloadLength;
} FlexrayReplayPacketHeader_t;


struct p_fr_keyslotcfg_t
{
    uint16_t keySlotId_1;
    uint8_t keySlotUsedForStartup_1;
    uint8_t keySlotUsedForSync_1;

    uint16_t keySlotId_2;
    uint8_t keySlotUsedForStartup_2;
    uint8_t keySlotUsedForSync_2;
};
typedef struct p_fr_keyslotcfg_t FRPhyNodeKeySlotFlat_t;


//定义函数返回值
typedef enum
{
	CMD_SUCCESS = 0,  //函数执行成功
	CMD_FAIL_STATUS,  //设备返回的状态失败
	CMD_FAIL_TIMEOUT, //函数执行超时，可能网络通讯问题
	CMD_FAIL_SOCKET,  //本地建立socket失败
	CMD_FAIL_EXIT,    //程序退出时接收函数可能的返回值
	CMD_FAIL_PARAMETER, //函数输入参数有误
    CMD_FAIL_INSTANCENOEXIST,//实例号不存在，UFr_Open应返回大于0的有效实例号
    CMD_FAIL_OPENFIBEX,   //打开Fibex数据库文件失败，可能由于文件格式有错误或Fibex版本不兼容
    CMD_FAIL_PROCEDURE,  //函数调用流程错误，需要首先调用UFr_Open函数并执行成功后再调用其他函数
    CMD_FAIL_INVALIDFILE, //无效文件
    CMD_FAIL_SET_NODE_ATTIBUTE,  //设置节点属性失败
    CMD_FAIL_SET_NODE_PARAMETER,  //设置节点参数失败
    CMD_FAIL_SET_CC_PARAMETER,    //设置CC控制器参数失败
    CMD_FAIL_SIGNAL_NO_EXIST,     //加载的数据库文件中不存在该信号的定义
    CMD_FAIL_NO_SPACE,            //要编码信号的缓冲区空间不足，可能由于缓冲区长度定义错误，或者信号定义有误
    CMD_FAIL_UNKNOW_SIGNAL,       //加载的fibex文件没有该信号的定义
    CMD_FAIL_UNKNOW_FRAME,        //加载的fibex文件没有该帧的定义
} CMD_RETURN_STATUS;

// 定义flexray节点所处的协议状态
typedef enum
{
	FR_POCSTATE_DEFAULT_CONFIG = 0,
	FR_POCSTATE_CONFIG,
	FR_POCSTATE_WAKEUP,
	FR_POCSTATE_READY,
	FR_POCSTATE_NORMAL_PASSIVE,
	FR_POCSTATE_NORMAL_ACTIVE,
	FR_POCSTATE_HALT,
	FR_POCSTATE_STARTUP,
} Fr_POC_state_type;

// 定义flexray节点WAKEUPSTATUS状态
typedef enum
{
    FrPSR0_WAKEUPSTATUS_UNDEFINED = 0,
    FrPSR0_WAKEUPSTATUS_RECEIVED_HEADER,
    FrPSR0_WAKEUPSTATUS_RECEIVED_WUP,
    FrPSR0_WAKEUPSTATUS_COLLISION_HEADER,
    FrPSR0_WAKEUPSTATUS_COLLISION_WUP,
    FrPSR0_WAKEUPSTATUS_COLLISION_UNKNOWN,
    FrPSR0_WAKEUPSTATUS_TRANSMITTED,
} Fr_WAKEUP_state_type;

typedef struct CANFrameHead
{
    uint8  channelNo;
    uint32 serial_number;
    uint32 timestamp_s;
    uint32 timestamp_us;
    uint32 canID;    
    uint8  protocolType; //0:CAN 2:CANFD  3:CANFD FAST    
    uint8  isExtendedFrame;
    uint8  isRTRFrame;
    uint8  payloadLength;
} uCANFrameHead_t;

typedef struct CANTxFrameHead
{
    uint32 canID;
    uint8  protocolType; //0:CAN 2:CANFD  3:CANFD FAST
    uint8  RFResponseFlag;//Remote frame response flag
    uint8  isExtendedFrame;
    uint8  isRTRFrame;
    uint8  payloadLength;
} uCANTxFrameHead_t;

typedef struct CANFDParamstruct{
    uint32 stdFrameRxId; /* Receive ID for standard frame */
    uint32 stdFrameRxIdMask; /* Receive ID mask for standard frame */
    uint32 extFrameRxId; /* Receive ID for extended frame */
    uint32 extFrameRxIdMask; /* Receive ID mask for extended frame */
    uint8_t reserve;    /*used for config MB max payload length, user can't cfg this value,default to 64 bytes*/
    /*
     * The bitrate settings for standard frames or for the arbitration phase of FD frames.
     */
    uint32 bitrate; /* The bitrate (bps) */
    /* Bit sampling point */
    /* User defined value (0 < bitSamplePoint < 1000), other value enable the automatically calculation */
    uint32 bitSamplePoint;
    /* Bit time parameter for CAN or CAN FD arbitration phase */
    /* To use following parameters if 'bitrate' field in this structure is set to 0 */
    uint16_t PRESDIV;    /* Prescaler Division factor setting */
    uint8_t PROPSEG;     /* Propagation Segment setting */
    uint8_t PSEG1;       /* Phase Segment 1 setting */
    uint8_t PSEG2;       /* Phase Segment 2 setting */
    uint8_t RJW;         /* ReSynchronization Jump Width setting */


    /*
     * The bitrate setting for the data phase of FD frames.
     */
    uint32 bitrateFD; /* The bitrate (bps) */
    /* Bit sampling point */
    /* User defined value (0 < bitSamplePoint < 1000), other value enable the automatically calculation */
    uint32 bitSamplePointFD;
    /* Bit time parameter for CAN FD data phase */
    /* To use following parameters if 'bitrateFD' field in this structure is set to 0 */
    uint16_t PRESDIV_FD; /* Prescaler Division factor setting */
    uint8_t PROPSEG_FD;  /* Propagation Segment setting */
    uint8_t PSEG1_FD;    /* Phase Segment 1 setting */
    uint8_t PSEG2_FD;    /* Phase Segment 2 setting */
    uint8_t RJW_FD;      /* ReSynchronization Jump Width setting */

    uint8_t autoRFR;    /* Automatic remote frame response */

    uint8_t disableTxFiFo; /*=1: disable tx FiFo, lost the frame when send fail*/
                           /*=0: enable  tx FiFo, retry send the frame when send fail*/
}tCANFDParamstruct;

enum CANFrameType{
    UnknownFrame = 0x0,
    DataFrame,
    ErrorFrame,
    RemoteRequestFrame,
    InvalidFrame
};

enum protocolType{
    CAN_TYPE = 0,
    CANFD_TYPE = 2,
    CANFDFAST_TYPE =3,
};

enum CANNodeID{
    CANNODE_0 = 0,
    CANNODE_1,
    CANNODE_2,
    CANNODE_3,
    CANNODE_4,
    CANNODE_5,
    CANNODE_6,
    CANNODE_7
};
#ifdef __cplusplus
extern "C" { /* using a C++ compiler */
#endif

	//typedef struct CommunicationLib CommunicationLibStruct; /* make the class opaque to the wrapper */


	//************************************************************************************
    //for flexray interface **************************************************************
	//************************************************************************************
	
    /**
    * @brief  打开flexray设备,应在所有函数执行前调用，打开成功后返回有效的实例号
    * @since  1.0.0
    * @param  FrIPaddr     flexray设备的IP地址
    * @return > 0为有效的设备实例号，否则打开失败
    */
    DLLIMPORT sint8 UFr_Open_Wrapper(char *FrIPaddr);
	
	/**
    * @brief  获得软件版本信息,包含动态库版本和设备固件版本,应在UFr_Open_Wrapper函数执行成功后调用才能获得所有信息，
    * 否则只能获得动态库的版本信息.
    * @since  2.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  dll_version 函数执行成功后保存动态库版本信息，16进制显示，例如2.0.0.1版本的数值为0x02000001
    * @param  firmware_version 函数执行成功后保存固件版本信息，16进制显示，例如2.0.0.1版本的数值为0x02000001
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT  uint8 UBus_get_software_version_Wrapper(sint8 instance, uint32 *dll_version, uint32 *firmware_version);
	
    /**
    * @brief  设置flexray设备的总线cluster参数
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  controller_num 控制器号，flexray接口的controller_num为0
    * @param  fcp     总线参数定义结构体，具体请参看 FlexrayClusterParameter_t 的定义
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UFr_Set_Cluster_Parameter_Wrapper(sint8 instance, uint8 controller_num, const FlexrayClusterParameter_t *fcp);

    /**
    * @brief  设置flexray设备的总线节点参数
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  controller_num 控制器号，flexray接口的controller_num为0
    * @param  fnp     节点参数定义结构体，具体请参看 FlexrayNodeParameter_t 的定义
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UFr_Set_Node_Parameter_Wrapper(sint8 instance, uint8 controller_num, const FlexrayNodeParameter_t *fnp);

    /**
    * @brief  设置flexray设备的通讯控制器参数
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  controller_num 控制器号，flexray接口的controller_num为0
    * @param  fccp     通讯控制器参数定义结构体，具体请参看 FlexrayCCParameter_t 的定义
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UFr_Set_CC_Parameter_Wrapper(sint8 instance, uint8 controller_num, const FlexrayCCParameter_t *fccp);

    /**
    * @brief  设置flexray设备的时隙参数
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  controller_num 控制器号，flexray接口的controller_num为0
    * @param  msgBufferParameter     时隙参数定义结构体，具体请参看 FlexrayMsgBufferParameter_t 的定义
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UFr_Set_Slot_Parameter_Wrapper(sint8 instance, uint8 controller_num, FlexrayMsgBufferParameter_t *msgBufferParameter);

    /**
    * @brief  设置flexray设备的Fifo参数
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  controller_num 控制器号，flexray接口的controller_num为0
    * @param  fifo_param_flat     FIFO参数定义结构体，具体请参看 FlerayFifoParameterFlat_t 的定义
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UFr_Set_Fifo_Parameter_Wrapper(sint8 instance, uint8 controller_num, FlexrayFifoParameterFlat_t *fifo_param_flat);

    /**
    * @brief  设置flexray设备的keyslot
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  controller_num 控制器号，flexray接口的controller_num为0
    * @param  pFRPhyNodeKeySlotFlat, reference to struct define of FRPhyNodeKeySlotFlat_t
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UFr_SetFrPhyNodeKeySlot_Wrapper(sint8 instance, uint8 controller_num, FRPhyNodeKeySlotFlat_t *pFRPhyNodeKeySlotFlat);

    /**
    * @brief  设置flexray设备的参数生效，即上面四个函数执行后需要调用本函数使参数设置生效
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  controller_num 控制器号，flexray接口的controller_num为0
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UFr_Configure_Node_Wrapper(sint8 instance, uint8 controller_num);

    /**
    * @brief  向FlexRay总线发送wakeup pattern，唤醒总线上处于sleep状态的节点，此函数只是使能发送唤醒操作，实际发送在启动节点时产生
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  controller_num     控制器号，flexray总线的控制器号为0
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UFr_SendWakeUp_Wrapper(sint8 instance, uint8 controller_num);

    /**
    * @brief  set flexray node work mode, support normal mode and playback mode
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  controller_num     控制器号，flexray总线的控制器号为0
    * @param  workmode   FR_WORKDMODE_NORMAL,FR_WORKDMODE_PLAYBACK
    * @param  MTSenable   Enable MTS
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UFr_Set_WorkMode_Wrapper(sint8 instance, uint8 controller_num, uint8 workmode, uint8 MTSenable);

	/**
    * @brief  设定一些不常用的设备参数
    * @since  2.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  controller_num     控制器号，flexray总线的控制器号为0
    * @param  ParameterIndex   配置参数枚举索引值，具体取值参考结构体Fr_PARAMETER的定义
    * @param  value   要配置的参数值
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UFr_WriteValue_Wrapper(sint8 instance, uint8 controller_num, Fr_PARAMETER ParameterIndex, uint32 value);

    /**
    * @brief  读取一些不常用的设备参数
    * @since  2.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  controller_num     控制器号，flexray总线的控制器号为0
    * @param  ParameterIndex   读取参数枚举索引值，具体取值参考结构体Fr_PARAMETER的定义
    * @param  value   函数执行成功后读取到的参数值
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UFr_ReadValue_Wrapper(sint8 instance, uint8 controller_num, Fr_PARAMETER ParameterIndex, uint32 *value);

    /**
    * @brief  启动flexray节点，建立总线连接
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  controller_num     控制器号，flexray总线的控制器号为0
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UFr_Start_Node_Wrapper(sint8 instance, uint8 controller_num);

    /**
    * @brief  启动接收数据上传操作
    *         是否建立总线连接可以通过UFr_GetFrNodeState函数获得。设备默认关闭接收数据上传功能。
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  controller_num     控制器号，flexray总线的控制器号为0
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UFr_StartFrRxUpload_Wrapper(sint8 instance, uint8 controller_num);

    /**
    * @brief  停止接收数据上传操作，一般在需要stop设备之前调用或不再需要处理接收数据时调用。
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  controller_num     控制器号，flexray总线的控制器号为0
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UFr_StopFrRxUpload_Wrapper(sint8 instance, uint8 controller_num);

    /**
    * @brief  获得flexray设备当前的协议状态，可在UFr_Start_Node函数执行后循环调用获取设备的总线协议状态
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  controller_num     控制器号，flexray总线的控制器号为0
    * @param  frnodeState        返回的协议状态，具体参看类型定义Fr_POC_state_type
    * @param  curCycle        返回总线当前的周期数
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UFr_GetFrNodeState_Wrapper(sint8 instance, uint8 controller_num, uint8 &frnodeState, uint8 &curCycle);

    /**
    * @brief  获得网络管理向量的长度和数据
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  controller_num     控制器号，flexray总线的控制器号为0
    * @param  NWVectorLength        返回网络管理向量的长度
    * @param  NWVector        返回网络管理向量数据，调用时应提供12个字节的数组指针
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UFr_GetFrNWVector_Wrapper(sint8 instance, uint8 controller_num, uint8 *NWVectorLength, uint8 *NWVector);

    /**
    * @brief  获得PSR寄存器状态值
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  controller_num     控制器号，flexray总线的控制器号为0
    * @param  frPSRState        返回PSR寄存器值，调用时应提供4个字的数组指针
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UFr_GetFrPSRState_Wrapper(sint8 instance, uint8 controller_num, uint16 *frPSRState);

    /**
    * @brief  获取动态库缓冲区包含的接收数据包个数
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @return 接收队列中的数据包个数
    */
    DLLIMPORT sint32 UFr_framesAvailable_Wrapper(sint8 instance);
	
	/**
    * @brief  清除FlexRay接口接收缓冲区
    * @since  1.0.1
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  controller_num     控制器号，flexray总线的控制器号为0
    * @return 参看 CMD_RETURN_STATUS 定义
    */
	DLLIMPORT uint8 UFr_ClearRcvBuffer_Wrapper(sint8 instance, uint8 controller_num);
	

    DLLIMPORT uint8 UFr_SetNodeRxTimeout_Wrapper(sint8 instance, uint8 controller_num, uint32 rxTimeout);

    /**
    * @brief  接收flexray消息，如果当前接收队列中没有消息，阻塞等待timeout_ms毫秒后退出
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  rcvFrFrameHead    接收flexray消息头的结构体地址，结构体定义参看 uFlexrayFrameHead_t 的定义
    * @param  rcvFrDatabuffer    接收flexray消息数据的缓冲区地址
    * @param  timeout_ms         超时等待时间，毫秒单位
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UFr_Receive_Wrapper(sint8 instance, uFlexrayFrameHead_t *rcvFrFrameHead, uint16 *rcvFrDatabuffer,  int timeout_ms);

    /**
    * @brief  打包回放flexray消息，1次可以发送多包FlexRay消息
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  controller_num     控制器号，flexray总线的控制器号为0
    * @param  numberOfSubPacket  指定本次发送缓冲区framebuffer包含多少帧FlexRay消息
    * @param  framebuffer        发送缓冲区指针,缓冲区中每帧数据头部分定义参考结构体 FlexrayReplayPacketHeader_t 的定义
    * @param  framebuffer_byteslength         发送缓冲区字节长度
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UFr_ReplaySend_Wrapper(sint8 instance, uint8 controller_num, uint8 numberOfSubPacket, uint8 *framebuffer, uint16 framebuffer_byteslength);

    /**
    * @brief  发送flexray消息
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  controller_num     控制器号，flexray总线的控制器号为0
    * @param  messagebuffer_id   消息缓冲区ID号，应由UFr_Set_Slot_Parameter函数返回的msgBufferID作为输入，或者用户自己定义
    * @param  buffer             发送数据的缓冲区首地址
    * @param  wordlength         发送数据字长度
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UFr_Transmit_Wrapper(sint8 instance, uint8 controller_num, uint8 messagebuffer_id,  uint16 *buffer,	uint8 wordlength);

	DLLIMPORT uint8 UFr_FillData_beforeTransmit_MultiFrame_Wrapper(sint8 instance, uint8 bufferindex, uint8 messagebuffer_id,  uint8 *buffer,	uint8 bytelength);
    DLLIMPORT uint8 UFr_Fibex_FillData_beforeTransmit_MultiFrame_Wrapper(sint8 instance, uint8 controller_num, const char *inFrameName, uint8 bufferindex,  uint8 *buffer,	uint8 bytelength);
    DLLIMPORT uint8 UFr_Transmit_MultiFrame_Wrapper(sint8 instance, uint8 numberOfSubPacket, uint8 *CountOfSent, uint8 WaitForACK);


    /**
    * @brief  停止flexray设备功能
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  controller_num     控制器号，flexray总线的控制器号为0
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UFr_Stop_Wrapper(sint8 instance, uint8 controller_num);

    /**
    * @brief  关闭flexray设备
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UFr_Close_Wrapper(sint8 instance);


	//************************************************************************************
    //for CAN interface*******************************************************************
	//************************************************************************************
	
	/**
    * @brief  打开CAN/CANFD设备通道,应该在进行所有CAN接口函数相关操作之前调用，打开成功后返回有效的实例号
    * @since  1.0.0
    * @param  FrIPaddr     flexray设备的IP地址
	* @param  nodeindex    CAN设备的通道号
    * @return > 0为有效的设备实例号，否则打开失败
    */
    DLLIMPORT sint8 UCAN_Open_Wrapper(char *FrIPaddr, uint8 nodeindex);

	/**
    * @brief  关闭CAN/CANFD设备
    * @since  1.0.0
    * @param  instance 执行UCAN_Open_Wrapper成功后返回的设备实例号
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT sint8 UCAN_Close_Wrapper(sint8 instance);

	/**
    * @brief  设置CAN/CANFD设备的通讯参数
    * @since  1.0.0
    * @param  instance 执行UCAN_Open_Wrapper成功后返回的设备实例号
    * @param  ptCANParamstruct   总线参数定义结构体指针，用于配置总线参数，调用前需要预先定义该结
	*         构体变量并赋值，具体请参看结构体 tCANFDParamstruct 的定义。
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UCAN_Configure_Wrapper(sint8 instance, tCANFDParamstruct *ptCANParamstruct);

	/**
    * @brief  启动CAN/CANFD设备,应该在配置操作完成后调用，启动通道后即可进行发送和接收操作。
    * @since  1.0.0
    * @param  instance 执行UCAN_Open_Wrapper成功后返回的设备实例号
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UCAN_Start_Wrapper(sint8 instance);

	/**
    * @brief  停止CAN/CANFD设备,应该在程序退出前调用，停止通道后不能进行发送和接收操作。
    * @since  1.0.0
    * @param  instance 执行UCAN_Open_Wrapper成功后返回的设备实例号
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UCAN_Stop_Wrapper(sint8 instance);
	
	/**
    * @brief  发送CAN/CANFD消息
    * @since  1.0.0
    * @param  instance 执行UCAN_Open_Wrapper成功后返回的设备实例号
    * @param  uCANTxFrameHead    发送报文的帧头结构体，参考定义 uCANTxFrameHead_t
    * @param  sendbuffer         发送数据的缓冲区首地址
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UCAN_Transmit_Wrapper(sint8 instance, uCANTxFrameHead_t uCANTxFrameHead,	uint8 *sendbuffer);

	/**
    * @brief  接收CAN/CANFD消息
    * @since  1.0.0
    * @param  instance 执行UCAN_Open_Wrapper成功后返回的设备实例号
    * @param  outCANFrameHead    接收报文的帧头结构体，参考定义 uCANFrameHead_t
    * @param  databuffer         接收数据的缓冲区首地址
	* @param  timeout_ms         阻塞接收的超时时间，单位ms，超时后退出函数
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UCAN_Receive_Wrapper(sint8 instance, uCANFrameHead_t &outCANFrameHead, uint8 *databuffer, int timeout_ms);

	/**
    * @brief  获得缓冲区中包含的接收数据包个数
    * @since  1.0.0
    * @param  instance 执行UCAN_Open_Wrapper成功后返回的设备实例号
    * @param  availableMsgCount  函数执行成功后返回缓冲区中未被应用软件接收的报文数量
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UCAN_framesAvailable_Wrapper(sint8 instance, sint32 *availableMsgCount);
	
	/**
    * @brief  清除接收缓冲区中未被应用程软件读取的CAN报文
    * @since  1.0.0
    * @param  instance 执行UCAN_Open_Wrapper成功后返回的设备实例号
    * @return 参看 CMD_RETURN_STATUS 定义
    */
	DLLIMPORT uint8 UCAN_ClearRcvBuffer_Wrapper(sint8 instance);

    DLLIMPORT uint8 UCAN_SetNodeRxTimeout_Wrapper(sint8 instance, uint32 rxTimeout);


    //for device
    /**
    * @brief  设置设备时间戳偏移，设备上电后自动开始计时，如果对时间戳有要求，可以调用此函数设置时间偏移
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  Second   偏移秒时间，输入0表示从0秒开始计时
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UBus_SetTimeOffset_Wrapper(sint8 instance, uint64 Second);


	
	//************************************************************************************
    //for Fibex file   *******************************************************************
	//************************************************************************************
    /**
    * @brief  用于打开fibex文件，执行此函数前需要成功执行UFr_Open_Wrapper函数，即已经成功打开设备
    * @since  1.0.0
    * @param  FibexFileName 需要解析的Fibex文件绝对路径
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT sint8 UFr_Fibex_Open(const char *FibexFileName);


    /**
    * @brief  关闭fibex文件，当程序不再使用fibex文件的信息时可以执行此函数关闭fibex文件释放系统资源
    * @since  1.0.0
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT sint8 UFr_Fibex_Close(void);


    /**
    * @brief  用于从fibex文件获取总线参数，需要用户输入cluster名称和ECU控制器名称字符串，
    *         函数如果返回正确，则可以使用outfcp，outfnp，outfccp对应的结构体进行后面的初始化操作。
    * @since  1.0.0
    * @param  inClusterName 用户输入cluster名称
    * @param  inControllerName 用户输入ECU控制器名称
    * @param  outfcp   函数返回的cluster参数
    * @param  outfnp   函数返回的节点参数
    * @param  outfccp   函数返回的通讯控制器参数
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT sint8 UFr_Fibex_GetParameter(const char *inClusterName, const char *inControllerName,
                            FlexrayClusterParameter_t *outfcp,FlexrayNodeParameter_t *outfnp, FlexrayCCParameter_t *outfccp);

    /**
    * @brief  通过信号名称查询该信号所在的帧名称，
    *         函数如果返回正确，则可以使用outFrameName进行后面时隙参数获得和配置时隙。
    * @since  1.0.0
    * @param  signalname 用户输入信号名称
    * @param  outFrameName 函数返回的该信号所在帧名称
    * @param  isFirstGet   函数输出参数，表明该帧名是否为第一次获得，用户可根据该参数返回值来判断是否需要配置该帧对应的时隙参数，
    *                      如果为true则可以进行配置，否则不需要再配置，可能之前该函数查询的某个信号已经在该帧中，不要多次配置相同帧的时隙参数
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT sint8 UFr_Fibex_GetFrameName_BySignalName(const char *signalname, char *outFrameName, bool *isFirstGet);


    /**
    * @brief  通过帧名称获得该帧的时隙配置信息，
    *         函数如果返回正确，则可以使用outmsgBufferParameter进行后面的UFr_Set_Slot_Parameter_Wrapper时隙参数配置。
    * @since  1.0.0
    * @param  inFrameName 用户输入帧名称
    * @param  isTx        指明该帧是否为发送帧
    * @param  PPIflag     指明该帧负载前导指示标志
    * @param  outmsgBufferParameter   函数输出参数，如果函数执行成功，该结构体存放该帧的时隙配置参数，
    *         可作为UFr_Set_Slot_Parameter_Wrapper函数的输入参数。
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT sint8 UFr_Fibex_GetSlotParameter(const char *inFrameName, bool isTx, bool PPIflag, FlexrayMsgBufferParameter_t *outmsgBufferParameter);

    /**
    * @brief  通过帧名称进行对应时隙的参数设置，合并了UFr_Fibex_GetSlotParameter和UFr_Set_Slot_Parameter_Wrapper两个函数的操作，
    *         函数如果返回正确，当该帧为发送帧时，可以使用UFr_Fibex_Transmit_Wrapper函数进行该帧的数据发送。
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  controller_num     控制器号，flexray总线的控制器号为0
    * @param  inFrameName 用户输入帧名称
    * @param  isTx        指明该帧是否为发送帧
    * @param  PPIflag     指明该帧负载前导指示标志
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UFr_Fibex_Set_Slot_Parameter_Wrapper(sint8 instance, uint8 controller_num, const char *inFrameName,
                                                         bool isTx, bool PPIflag);

    /**
    * @brief  通过帧名称进行该帧的数据发送操作。
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  controller_num     控制器号，flexray总线的控制器号为0
    * @param  inFrameName 用户输入帧名称
    * @param  buffer        发送数据缓冲区字节数组首地址
    * @param  wordlength    发送数据字节长度
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UFr_Fibex_Transmit_Wrapper(sint8 instance, uint8 controller_num, const char *inFrameName,  uint8 *buffer,	uint8 bytelength);

	/**
    * @brief  接收flexray消息并解析出该帧的帧名称，如果当前接收队列中没有消息，阻塞等待timeout_ms毫秒后退出
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  rcvFrFrameHead    接收flexray消息头的结构体地址，结构体定义参看 uFlexrayFrameHead_t 的定义
    * @param  rcvFrDatabuffer   接收flexray消息数据的缓冲区地址
	* @param  outFrameName      函数返回该帧在fibex文件中定义的帧名称
    * @param  timeout_ms        超时等待时间，毫秒单位
    * @return 参看 CMD_RETURN_STATUS 定义
    */
	DLLIMPORT uint8 UFr_Fibex_Receive_Wrapper(sint8 instance, uFlexrayFrameHead_t *rcvFrFrameHead, uint8 *rcvFrDatabuffer, char *outFrameName, int timeout_ms);

    /**
    * @brief  通过接收到的FlexRay帧头信息，获得该帧的帧名称。
    * @since  1.0.0
    * @param  rcvFrFrameHead 一般将接收函数返回的帧头信息作为这个输入参数
    * @param  outFrameName  函数返回该帧在fibex文件中定义的帧名称
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT sint8 UFr_Fibex_GetFrameName(uFlexrayFrameHead_t *rcvFrFrameHead,  char *outFrameName);

    /**
    * @brief  解码输入帧中的指定信号物理值。
    * @since  1.0.0
    * @param  rcvFrFrameHead 一般将接收函数返回的帧头信息作为这个输入参数
    * @param  rcvFrDatabuffer 一般将接收函数返回的帧数据信息作为这个输入参数
    * @param  signalname  输入要解码的信号名称
    * @param  signalval   如果函数执行成功，函数返回该信号的物理值，所有物理值均采用float型表示
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT sint8 UFr_Fibex_decodeSignal(uFlexrayFrameHead_t *rcvFrFrameHead, uint8 *rcvFrDatabuffer, const char *signalname, float *signalval);

    /**
    * @brief  编码输出帧中的指定信号物理值。函数执行成功后，需要调用UFr_Fibex_Transmit_Wrapper或UFr_Transmit_Wrapper函数将DataBuffer指定的数据发出
    * @since  1.0.0
    * @param  signalname  输入要编码的信号名称
    * @param  signalval   输入要编码的信号物理值
    * @param  DataBuffer  输入要发送的数据帧数据部分的首地址
    * @param  DataBufferBytelength 指定数据帧数据部分缓冲区的字节长度
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT sint8 UFr_Fibex_encodeSignal(const char *signalname, const float signalval, uint8 *DataBuffer, uint8 DataBufferBytelength);

//************************************************************************************
    //for AutoSar file   *******************************************************************
	//************************************************************************************
    /**
    * @brief  用于打开AutoSar文件，执行此函数前需要成功执行UFr_Open_Wrapper函数，即已经成功打开设备
    * @since  1.0.0
    * @param  autosarFileName 需要解析的Fibex文件绝对路径
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT sint8 UFr_AutoSar_Open(const char *autosarFileName);


    /**
    * @brief  关闭AutoSar文件，当程序不再使用AutoSar文件的信息时可以执行此函数关闭AutoSar文件释放系统资源
    * @since  1.0.0
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT sint8 UFr_AutoSar_Close(void);


    /**
    * @brief  用于从AutoSar文件获取总线参数，需要用户输入cluster名称和ECU控制器名称字符串，
    *         函数如果返回正确，则可以使用outfcp，outfnp，outfccp对应的结构体进行后面的初始化操作。
    * @since  1.0.0
    * @param  inClusterName 用户输入cluster名称
    * @param  inControllerName 用户输入ECU控制器名称
    * @param  outfcp   函数返回的cluster参数
    * @param  outfnp   函数返回的节点参数
    * @param  outfccp   函数返回的通讯控制器参数
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT sint8 UFr_AutoSar_GetParameter(const char *inClusterName, const char *inControllerName,
                            FlexrayClusterParameter_t *outfcp,FlexrayNodeParameter_t *outfnp, FlexrayCCParameter_t *outfccp);

    /**
    * @brief  通过信号名称查询该信号所在的帧名称，
    *         函数如果返回正确，则可以使用outFrameName进行后面时隙参数获得和配置时隙。
    * @since  1.0.0
    * @param  signalname 用户输入信号名称
    * @param  outFrameName 函数返回的该信号所在帧名称
    * @param  isFirstGet   函数输出参数，表明该帧名是否为第一次获得，用户可根据该参数返回值来判断是否需要配置该帧对应的时隙参数，
    *                      如果为true则可以进行配置，否则不需要再配置，可能之前该函数查询的某个信号已经在该帧中，不要多次配置相同帧的时隙参数
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT sint8 UFr_AutoSar_GetFrameName_BySignalName(const char *signalname, char *outFrameName, bool *isFirstGet);


    /**
    * @brief  通过帧名称获得该帧的时隙配置信息，
    *         函数如果返回正确，则可以使用outmsgBufferParameter进行后面的UFr_Set_Slot_Parameter_Wrapper时隙参数配置。
    * @since  1.0.0
    * @param  inFrameName 用户输入帧名称
    * @param  isTx        指明该帧是否为发送帧
    * @param  PPIflag     指明该帧负载前导指示标志
    * @param  outmsgBufferParameter   函数输出参数，如果函数执行成功，该结构体存放该帧的时隙配置参数，
    *         可作为UFr_Set_Slot_Parameter_Wrapper函数的输入参数。
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT sint8 UFr_AutoSar_GetSlotParameter(const char *inFrameName, bool isTx, bool PPIflag, FlexrayMsgBufferParameter_t *outmsgBufferParameter);

    /**
    * @brief  通过帧名称进行对应时隙的参数设置，合并了UFr_Fibex_GetSlotParameter和UFr_Set_Slot_Parameter_Wrapper两个函数的操作，
    *         函数如果返回正确，当该帧为发送帧时，可以使用UFr_Fibex_Transmit_Wrapper函数进行该帧的数据发送。
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  controller_num     控制器号，flexray总线的控制器号为0
    * @param  inFrameName 用户输入帧名称
    * @param  isTx        指明该帧是否为发送帧
    * @param  PPIflag     指明该帧负载前导指示标志
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UFr_AutoSar_Set_Slot_Parameter_Wrapper(sint8 instance, uint8 controller_num, const char *inFrameName,
                                                         bool isTx, bool PPIflag);

    /**
    * @brief  通过帧名称进行该帧的数据发送操作。
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  controller_num     控制器号，flexray总线的控制器号为0
    * @param  inFrameName 用户输入帧名称
    * @param  buffer        发送数据缓冲区字节数组首地址
    * @param  wordlength    发送数据字节长度
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT uint8 UFr_AutoSar_Transmit_Wrapper(sint8 instance, uint8 controller_num, const char *inFrameName,  uint8 *buffer,	uint8 bytelength);

	/**
    * @brief  接收flexray消息并解析出该帧的帧名称，如果当前接收队列中没有消息，阻塞等待timeout_ms毫秒后退出
    * @since  1.0.0
    * @param  instance 执行UFr_Open_Wrapper成功后返回的设备实例号
    * @param  rcvFrFrameHead    接收flexray消息头的结构体地址，结构体定义参看 uFlexrayFrameHead_t 的定义
    * @param  rcvFrDatabuffer   接收flexray消息数据的缓冲区地址
	* @param  outFrameName      函数返回该帧在fibex文件中定义的帧名称
    * @param  timeout_ms        超时等待时间，毫秒单位
    * @return 参看 CMD_RETURN_STATUS 定义
    */
	DLLIMPORT uint8 UFr_AutoSar_Receive_Wrapper(sint8 instance, uFlexrayFrameHead_t *rcvFrFrameHead, uint8 *rcvFrDatabuffer, char *outFrameName, int timeout_ms);

    /**
    * @brief  通过接收到的FlexRay帧头信息，获得该帧的帧名称。
    * @since  1.0.0
    * @param  rcvFrFrameHead 一般将接收函数返回的帧头信息作为这个输入参数
    * @param  outFrameName  函数返回该帧在fibex文件中定义的帧名称
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT sint8 UFr_AutoSar_GetFrameName(uFlexrayFrameHead_t *rcvFrFrameHead,  char *outFrameName);

    /**
    * @brief  解码输入帧中的指定信号物理值。
    * @since  1.0.0
    * @param  rcvFrFrameHead 一般将接收函数返回的帧头信息作为这个输入参数
    * @param  rcvFrDatabuffer 一般将接收函数返回的帧数据信息作为这个输入参数
    * @param  signalname  输入要解码的信号名称
    * @param  signalval   如果函数执行成功，函数返回该信号的物理值，所有物理值均采用float型表示
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT sint8 UFr_AutoSar_decodeSignal(uFlexrayFrameHead_t *rcvFrFrameHead, uint8 *rcvFrDatabuffer, const char *signalname, float *signalval);

    /**
    * @brief  编码输出帧中的指定信号物理值。函数执行成功后，需要调用UFr_Fibex_Transmit_Wrapper或UFr_Transmit_Wrapper函数将DataBuffer指定的数据发出
    * @since  1.0.0
    * @param  signalname  输入要编码的信号名称
    * @param  signalval   输入要编码的信号物理值
    * @param  DataBuffer  输入要发送的数据帧数据部分的首地址
    * @param  DataBufferBytelength 指定数据帧数据部分缓冲区的字节长度
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT sint8 UFr_AutoSar_encodeSignal(const char *signalname, const float signalval, uint8 *DataBuffer, uint8 DataBufferBytelength);
	
    /**
    * @brief  获得autosar文件中定义的所有clustername。
    * @since  2.0.0.0
    * @param  outBuffer 函数执行成功后将获取到的clustername存储用户分配的outBuffer数组中，存在多个时以分号';'分隔
    * @param  outBufferLength  用户提供的outBuffer缓冲区字节长度
    * @param  clusterCntTotal  autosar文件中定义的cluster总数
    * @param  clusterCntGetted 本次函数执行获得name数量
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT sint8 UFr_AutoSar_GetAllClusterName(char *outBuffer, uint32 outBufferLength, int *clusterCntTotal, int *clusterCntGetted);

    /**
    * @brief  获得autosar文件中定义的所有ECUrname。
    * @since  2.0.0.0
    * @param  outBuffer 函数执行成功后将获取到的ECUname存储用户分配的outBuffer数组中，存在多个时以分号';'分隔
    * @param  outBufferLength  用户提供的outBuffer缓冲区字节长度
    * @param  ecuCntTotal  autosar文件中定义的ECU总数
    * @param  ecuCntGetted 本次函数执行获得name数量
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT sint8 UFr_AutoSar_GetAllECUName(char *outBuffer, uint32 outBufferLength, int *ecuCntTotal, int *ecuCntGetted);

    /**
    * @brief  通过clustername获得autosar文件中定义的该cluster下的所有FrameName。
    * @since  2.0.0.0
    * @param  clustername 用户输入的clustername
    * @param  outBuffer 函数执行成功后将获取到的framename存储到用户分配的outBuffer数组中，存在多个时以分号';'分隔
    * @param  outBufferLength  用户提供的outBuffer缓冲区字节长度
    * @param  frameCntTotal  用户输入的clustername中定义的frame总数
    * @param  frameCntGetted 本次函数执行获得frame数量
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT sint8 UFr_AutoSar_GetAllFrameName_ByClusterName(const char *clustername, char *outBuffer, uint32 outBufferLength, int *frameCntTotal, int *frameCntGetted);

    /**
    * @brief  通过FrameName获得autosar文件中定义的该Frame下的所有Signal。
    * @since  2.0.0.0
    * @param  framename 用户输入的framename
    * @param  outBuffer 函数执行成功后将获取到的SignalName存储到用户分配的outBuffer数组中，存在多个时以分号';'分隔
    * @param  outBufferLength  用户提供的outBuffer缓冲区字节长度
    * @param  signalCntTotal  用户输入的FrameName中定义的Signal总数
    * @param  signalCntGetted 本次函数执行获得signal数量
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT sint8 UFr_AutoSar_GetAllSignalName_ByFrameName(const char *framename, char *outBuffer, uint32 outBufferLength, int *signalCntTotal, int *signalCntGetted);

    /**
    * @brief  通过ECUName获得autosar文件中定义的该ECUName下的所有FrameName。
    * @since  2.0.0.0
    * @param  ECUname 用户输入的ECUname
    * @param  outBuffer 函数执行成功后将获取到的SignalName存储到用户分配的outBuffer数组中，存在多个时以分号';'分隔
    * @param  outBufferLength  用户提供的outBuffer缓冲区字节长度
    * @param  frameCntTotal  用户输入的ECUname中定义的frame总数
    * @param  frameCntGetted 本次函数执行获得frame数量
    * @param  isTx 标识此次获得的是发送帧还是接收帧
    * @return 参看 CMD_RETURN_STATUS 定义
    */
    DLLIMPORT sint8 UFr_AutoSar_GetAllFrameName_ByECUName(const char *ECUname, char *outBuffer, uint32 outBufferLength, int *frameCntTotal, int *frameCntGetted, bool isTx);





#ifdef __cplusplus
}
#endif

#endif // UBUSCOMMLIBWRAPPER_H
