
#ifndef DIO_CFG_H
#define DIO_CFG_H
/*
 Notes:
 - The runtime configuration is handled by the Port Driver Module.
 - The configuration and usage of the DIO Driver Module is adapted to
   the microcontroller and ECU.

*/

/*Version check macros */
#define DIO_AR_RELEASE_MAJOR_VERSION     (4U)
#define DIO_AR_RELEASE_MINOR_VERSION     (4U)
#define DIO_AR_RELEASE_REVISION_VERSION  (0U)


#define DIO_SW_MAJOR_VERSION  (0U)
#define DIO_SW_MINOR_VERSION  (1U)
#define DIO_SW_PATCH_VERSION  (0U)

/*******************************************************************************
**                      Includes                                              **
*******************************************************************************/

/*******************************************************************************
**                      Global Macro Definitions                              **
*******************************************************************************/

/*                          Container: DioGeneral                            */
/*

  Pre-processor switch to enable/disable detection of Development errors.
  - if defined ON, Development error detection is enabled
  - if defined OFF, Development error detection is disabled
*/
#define DIO_DEV_ERROR_DETECT                (STD_OFF)

/*
  Pre-Compiler switch to include the API Dio_GetVersionInfo()
  - if defined ON, Dio_GetVersionInfo API available
  - if defined OFF, Dio_GetVersionInfo API not available
*/
#define DIO_VERSION_INFO_API                (STD_ON)

/*
  Pre-Compiler switch to include the API Dio_FlipChannel()
  - if defined ON, Dio_FlipChannel API available
  - if defined OFF, Dio_FlipChannel API not available
*/
#define DIO_FLIP_CHANNEL_API                (STD_ON)

/*
  Pre-Compiler switch to include the Safety Check
*/
#define DIO_SAFETY_ENABLE                (STD_OFF)

/* Macro to define the maximum port available */
#define MAX_AVAILABLE_PORT                  (5U)

/* Values to mask the undefined port pins within a port */
#define DIO_MASK_ALL_PINS_PORT0             (0xffffffffU)
#define DIO_MASK_ALL_PINS_PORT1             (0xffffffffU)
#define DIO_MASK_ALL_PINS_PORT2             (0xffffffffU)
#define DIO_MASK_ALL_PINS_PORT3             (0xffffffffU)
#define DIO_MASK_ALL_PINS_PORT4             (0x0fffffffU)

/* Macro to define the No of channel groups configured */
#define DIO_CHANNELGROUPCOUNT               (0U)


/*******************************************************************************
**                           Global Symbols                                   **
*******************************************************************************/

/*
 Symbolic names for DIO Channels, Port & Channel groups
*/

/*
                       Symbolic names for Channels
*/
#define DIO_CHANNEL_0                    ((Dio_ChannelType)0)
#define DIO_CHANNEL_1                    ((Dio_ChannelType)1)
#define DIO_CHANNEL_2                    ((Dio_ChannelType)2)
#define DIO_CHANNEL_3                    ((Dio_ChannelType)3)
#define DIO_CHANNEL_4                    ((Dio_ChannelType)4)
#define DIO_CHANNEL_5                    ((Dio_ChannelType)5)
#define DIO_CHANNEL_6                    ((Dio_ChannelType)6)
#define DIO_CHANNEL_7                    ((Dio_ChannelType)7)
#define DIO_CHANNEL_8                    ((Dio_ChannelType)8)
#define DIO_CHANNEL_9                    ((Dio_ChannelType)9)
#define DIO_CHANNEL_10                    ((Dio_ChannelType)10)
#define DIO_CHANNEL_11                    ((Dio_ChannelType)11)
#define DIO_CHANNEL_12                    ((Dio_ChannelType)12)
#define DIO_CHANNEL_13                    ((Dio_ChannelType)13)
#define DIO_CHANNEL_14                    ((Dio_ChannelType)14)
#define DIO_CHANNEL_15                    ((Dio_ChannelType)15)
#define DIO_CHANNEL_16                    ((Dio_ChannelType)16)
#define DIO_CHANNEL_17                    ((Dio_ChannelType)17)
#define DIO_CHANNEL_18                    ((Dio_ChannelType)18)
#define DIO_CHANNEL_19                    ((Dio_ChannelType)19)
#define DIO_CHANNEL_20                    ((Dio_ChannelType)20)
#define DIO_CHANNEL_21                    ((Dio_ChannelType)21)
#define DIO_CHANNEL_22                    ((Dio_ChannelType)22)
#define DIO_CHANNEL_23                    ((Dio_ChannelType)23)
#define DIO_CHANNEL_24                    ((Dio_ChannelType)24)
#define DIO_CHANNEL_25                    ((Dio_ChannelType)25)
#define DIO_CHANNEL_26                    ((Dio_ChannelType)26)
#define DIO_CHANNEL_27                    ((Dio_ChannelType)27)
#define DIO_CHANNEL_28                    ((Dio_ChannelType)28)
#define DIO_CHANNEL_29                    ((Dio_ChannelType)29)
#define DIO_CHANNEL_30                    ((Dio_ChannelType)30)
#define DIO_CHANNEL_31                    ((Dio_ChannelType)31)
#define DIO_CHANNEL_32                    ((Dio_ChannelType)32)
#define DIO_CHANNEL_33                    ((Dio_ChannelType)33)
#define DIO_CHANNEL_34                    ((Dio_ChannelType)34)
#define DIO_CHANNEL_35                    ((Dio_ChannelType)35)
#define DIO_CHANNEL_36                    ((Dio_ChannelType)36)
#define DIO_CHANNEL_37                    ((Dio_ChannelType)37)
#define DIO_CHANNEL_38                    ((Dio_ChannelType)38)
#define DIO_CHANNEL_39                    ((Dio_ChannelType)39)
#define DIO_CHANNEL_40                    ((Dio_ChannelType)40)
#define DIO_CHANNEL_41                    ((Dio_ChannelType)41)
#define DIO_CHANNEL_42                    ((Dio_ChannelType)42)
#define DIO_CHANNEL_43                    ((Dio_ChannelType)43)
#define DIO_CHANNEL_44                    ((Dio_ChannelType)44)
#define DIO_CHANNEL_45                    ((Dio_ChannelType)45)
#define DIO_CHANNEL_46                    ((Dio_ChannelType)46)
#define DIO_CHANNEL_47                    ((Dio_ChannelType)47)
#define DIO_CHANNEL_48                    ((Dio_ChannelType)48)
#define DIO_CHANNEL_49                    ((Dio_ChannelType)49)
#define DIO_CHANNEL_50                    ((Dio_ChannelType)50)
#define DIO_CHANNEL_51                    ((Dio_ChannelType)51)
#define DIO_CHANNEL_52                    ((Dio_ChannelType)52)
#define DIO_CHANNEL_53                    ((Dio_ChannelType)53)
#define DIO_CHANNEL_54                    ((Dio_ChannelType)54)
#define DIO_CHANNEL_55                    ((Dio_ChannelType)55)
#define DIO_CHANNEL_56                    ((Dio_ChannelType)56)
#define DIO_CHANNEL_57                    ((Dio_ChannelType)57)
#define DIO_CHANNEL_58                    ((Dio_ChannelType)58)
#define DIO_CHANNEL_59                    ((Dio_ChannelType)59)
#define DIO_CHANNEL_60                    ((Dio_ChannelType)60)
#define DIO_CHANNEL_61                    ((Dio_ChannelType)61)
#define DIO_CHANNEL_62                    ((Dio_ChannelType)62)
#define DIO_CHANNEL_63                    ((Dio_ChannelType)63)
#define DIO_CHANNEL_64                    ((Dio_ChannelType)64)
#define DIO_CHANNEL_65                    ((Dio_ChannelType)65)
#define DIO_CHANNEL_66                    ((Dio_ChannelType)66)
#define DIO_CHANNEL_67                    ((Dio_ChannelType)67)
#define DIO_CHANNEL_68                    ((Dio_ChannelType)68)
#define DIO_CHANNEL_69                    ((Dio_ChannelType)69)
#define DIO_CHANNEL_70                    ((Dio_ChannelType)70)
#define DIO_CHANNEL_71                    ((Dio_ChannelType)71)
#define DIO_CHANNEL_72                    ((Dio_ChannelType)72)
#define DIO_CHANNEL_73                    ((Dio_ChannelType)73)
#define DIO_CHANNEL_74                    ((Dio_ChannelType)74)
#define DIO_CHANNEL_75                    ((Dio_ChannelType)75)
#define DIO_CHANNEL_76                    ((Dio_ChannelType)76)
#define DIO_CHANNEL_77                    ((Dio_ChannelType)77)
#define DIO_CHANNEL_78                    ((Dio_ChannelType)78)
#define DIO_CHANNEL_79                    ((Dio_ChannelType)79)
#define DIO_CHANNEL_80                    ((Dio_ChannelType)80)
#define DIO_CHANNEL_81                    ((Dio_ChannelType)81)
#define DIO_CHANNEL_82                    ((Dio_ChannelType)82)
#define DIO_CHANNEL_83                    ((Dio_ChannelType)83)
#define DIO_CHANNEL_84                    ((Dio_ChannelType)84)
#define DIO_CHANNEL_85                    ((Dio_ChannelType)85)
#define DIO_CHANNEL_86                    ((Dio_ChannelType)86)
#define DIO_CHANNEL_87                    ((Dio_ChannelType)87)
#define DIO_CHANNEL_88                    ((Dio_ChannelType)88)
#define DIO_CHANNEL_89                    ((Dio_ChannelType)89)
#define DIO_CHANNEL_90                    ((Dio_ChannelType)90)
#define DIO_CHANNEL_91                    ((Dio_ChannelType)91)
#define DIO_CHANNEL_92                    ((Dio_ChannelType)92)
#define DIO_CHANNEL_93                    ((Dio_ChannelType)93)
#define DIO_CHANNEL_94                    ((Dio_ChannelType)94)
#define DIO_CHANNEL_95                    ((Dio_ChannelType)95)
#define DIO_CHANNEL_96                    ((Dio_ChannelType)96)
#define DIO_CHANNEL_97                    ((Dio_ChannelType)97)
#define DIO_CHANNEL_98                    ((Dio_ChannelType)98)
#define DIO_CHANNEL_99                    ((Dio_ChannelType)99)
#define DIO_CHANNEL_100                    ((Dio_ChannelType)100)
#define DIO_CHANNEL_101                    ((Dio_ChannelType)101)
#define DIO_CHANNEL_102                    ((Dio_ChannelType)102)
#define DIO_CHANNEL_103                    ((Dio_ChannelType)103)
#define DIO_CHANNEL_104                    ((Dio_ChannelType)104)
#define DIO_CHANNEL_105                    ((Dio_ChannelType)105)
#define DIO_CHANNEL_106                    ((Dio_ChannelType)106)
#define DIO_CHANNEL_107                    ((Dio_ChannelType)107)
#define DIO_CHANNEL_108                    ((Dio_ChannelType)108)
#define DIO_CHANNEL_109                    ((Dio_ChannelType)109)
#define DIO_CHANNEL_110                    ((Dio_ChannelType)110)
#define DIO_CHANNEL_111                    ((Dio_ChannelType)111)
#define DIO_CHANNEL_112                    ((Dio_ChannelType)112)
#define DIO_CHANNEL_113                    ((Dio_ChannelType)113)
#define DIO_CHANNEL_114                    ((Dio_ChannelType)114)
#define DIO_CHANNEL_115                    ((Dio_ChannelType)115)
#define DIO_CHANNEL_116                    ((Dio_ChannelType)116)
#define DIO_CHANNEL_117                    ((Dio_ChannelType)117)
#define DIO_CHANNEL_118                    ((Dio_ChannelType)118)
#define DIO_CHANNEL_119                    ((Dio_ChannelType)119)
#define DIO_CHANNEL_120                    ((Dio_ChannelType)120)
#define DIO_CHANNEL_121                    ((Dio_ChannelType)121)
#define DIO_CHANNEL_122                    ((Dio_ChannelType)122)
#define DIO_CHANNEL_123                    ((Dio_ChannelType)123)
#define DIO_CHANNEL_124                    ((Dio_ChannelType)124)
#define DIO_CHANNEL_125                    ((Dio_ChannelType)125)
#define DIO_CHANNEL_126                    ((Dio_ChannelType)126)
#define DIO_CHANNEL_127                    ((Dio_ChannelType)127)
#define DIO_CHANNEL_128                    ((Dio_ChannelType)128)
#define DIO_CHANNEL_129                    ((Dio_ChannelType)129)
#define DIO_CHANNEL_130                    ((Dio_ChannelType)130)
#define DIO_CHANNEL_131                    ((Dio_ChannelType)131)
#define DIO_CHANNEL_132                    ((Dio_ChannelType)132)
#define DIO_CHANNEL_133                    ((Dio_ChannelType)133)
#define DIO_CHANNEL_134                    ((Dio_ChannelType)134)
#define DIO_CHANNEL_135                    ((Dio_ChannelType)135)
#define DIO_CHANNEL_136                    ((Dio_ChannelType)136)
#define DIO_CHANNEL_137                    ((Dio_ChannelType)137)
#define DIO_CHANNEL_138                    ((Dio_ChannelType)138)
#define DIO_CHANNEL_139                    ((Dio_ChannelType)139)
#define DIO_CHANNEL_140                    ((Dio_ChannelType)140)
#define DIO_CHANNEL_141                    ((Dio_ChannelType)141)
#define DIO_CHANNEL_142                    ((Dio_ChannelType)142)
#define DIO_CHANNEL_143                    ((Dio_ChannelType)143)
#define DIO_CHANNEL_144                    ((Dio_ChannelType)144)
#define DIO_CHANNEL_145                    ((Dio_ChannelType)145)
#define DIO_CHANNEL_146                    ((Dio_ChannelType)146)
#define DIO_CHANNEL_147                    ((Dio_ChannelType)147)
#define DIO_CHANNEL_148                    ((Dio_ChannelType)148)
#define DIO_CHANNEL_149                    ((Dio_ChannelType)149)
#define DIO_CHANNEL_150                    ((Dio_ChannelType)150)
#define DIO_CHANNEL_151                    ((Dio_ChannelType)151)
#define DIO_CHANNEL_152                    ((Dio_ChannelType)152)
#define DIO_CHANNEL_153                    ((Dio_ChannelType)153)
#define DIO_CHANNEL_154                    ((Dio_ChannelType)154)
#define DIO_CHANNEL_155                    ((Dio_ChannelType)155)

/*
                      Symbolic names for DIO ports
*/
#define DIO_PORT_0                          ((Dio_PortType)0)
#define DIO_PORT_1                          ((Dio_PortType)1)
#define DIO_PORT_2                          ((Dio_PortType)2)
#define DIO_PORT_3                          ((Dio_PortType)3)
#define DIO_PORT_4                          ((Dio_PortType)4)



/*******************************************************************************
**                      Global Constant Declarations                          **
*******************************************************************************/


//extern const struct Dio_ConfigType Dio_Config;

/*******************************************************************************
**                      Global Data Type                                      **
*******************************************************************************/

/* End of DIO_CFG_H */
#endif
