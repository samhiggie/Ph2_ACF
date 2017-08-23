<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="html" indent="yes"/>

    <!--now parse the whole thing-->
    <xsl:template match="/*">
        <html>
            <!--<form method="post">-->
        <!--<form>-->
            <!--<input type="submit" id="btn_sub" name="btn_sub" value="Submit" />-->
            <!--<input type="reset" id="btn_res" name="btn_res" value="Reset" />-->
            <head>
                <!--<script type="text/javascript" src="http://code.jquery.com/jquery.min.js"></script>-->
                <script type="text/javascript">
                 function SelectFile(val, Id){
                    var element=document.getElementById('configfile'+Id.toString());
                    if(val=='other')
                      element.style.display='inline';
                    else  
                      element.style.display='none';
                }

                function DisplayFields(val, position){
                    var Register = document.getElementById('conddata_Register_'+position.toString());
                    var RegisterLabel = document.getElementById('conddata_RegisterLabel_'+position.toString());
                    var FeId = document.getElementById('conddata_FeId_'+position.toString());
                    var FeIdLabel = document.getElementById('conddata_FeIdLabel_'+position.toString());
                    var CbcId = document.getElementById('conddata_CbcId_'+position.toString());
                    var CbcIdLabel = document.getElementById('conddata_CbcIdLabel_'+position.toString());
                    var UID = document.getElementById('conddata_UID_'+position.toString());
                    var UIDLabel = document.getElementById('conddata_UIDLabel_'+position.toString());
                    var Sensor = document.getElementById('conddata_Sensor_'+position.toString());
                    var SensorLabel = document.getElementById('conddata_SensorLabel_'+position.toString());
                    var Value = document.getElementById('conddata_Value_'+position.toString());
                    var ValueLabel = document.getElementById('conddata_ValueLabel_'+position.toString());

                    Register.style.display='none';
                    RegisterLabel.style.display='none';
                    FeId.style.display='none';
                    FeIdLabel.style.display='none';
                    CbcId.style.display='none';
                    CbcIdLabel.style.display='none';
                    UID.style.display='none';
                    UIDLabel.style.display='none';
                    Sensor.style.display='none';
                    SensorLabel.style.display='none';
                    Value.style.display='none';
                    ValueLabel.style.display='none';

                    if(val=="I2C")
                    {
                        Register.style.display='inline';
                        RegisterLabel.style.display='inline';
                        FeId.style.display='inline';
                        FeIdLabel.style.display='inline';
                        CbcId.style.display='inline';
                        CbcIdLabel.style.display='inline';
                    }
                    else if(val=="User")
                    {
                        UID.style.display='inline';
                        UIDLabel.style.display='inline';
                        FeId.style.display='inline';
                        FeIdLabel.style.display='inline';
                        CbcId.style.display='inline';
                        CbcIdLabel.style.display='inline';
                        Value.style.display='inline';
                        ValueLabel.style.display='inline';

                    }
                    else if(val=="HV")
                    {
                        FeId.style.display='inline';
                        FeIdLabel.style.display='inline';
                        Sensor.style.display='inline';
                        SensorLabel.style.display='inline';
                        Value.style.display='inline';
                        ValueLabel.style.display='inline';
                    }
                    else if(val=="TDC")
                    {
                        FeId.style.display='inline';
                        FeIdLabel.style.display='inline';
                    }
                    else
                    {
                        Register.style.display='none';
                        RegisterLabel.style.display='none';
                        FeId.style.display='none';
                        FeIdLabel.style.display='none';
                        CbcId.style.display='none';
                        CbcIdLabel.style.display='none';
                        UID.style.display='none';
                        UIDLabel.style.display='none';
                        Sensor.style.display='none';
                        SensorLabel.style.display='none';
                        Value.style.display='none';
                        ValueLabel.style.display='none';
                    }
                }

                function DisplayFieldsOnload(){
                    var ConditionDataNumber = <xsl:value-of select="count(*/SLink/ConditionData)"/>;
                    for(i=1; i&lt;ConditionDataNumber+1; i++) {
                        var elem = document.getElementById('conddata_'+i);
                        if(elem != null)
                        {
                            console.log(elem)
                            var selected_node = elem.options[elem.selectedIndex].value;
                            DisplayFields(selected_node, i);
                        }
                    }
                }
                
                 window.onload = DisplayFieldsOnload();
                </script> 
                <!--<link rel="stylesheet" type="text/css" href="misc/HWStyle.css"/>-->
            </head>
            <body onload='DisplayFieldsOnload();'>
                <h3>HWDescription</h3>
                <xsl:apply-templates select="BeBoard"/>
            </body>
        <!--</form>-->
        </html>
    </xsl:template>

    <!--Template for BeBoard nodes-->
    <xsl:template match="BeBoard">
        <div class="BeBoard">
        <h4> BeBoard Id: <xsl:value-of select="@Id"/> </h4>
            <!--<li>BoardType: <xsl:value-of select="@boardType"/></td>-->
            <div class="General">
            <li>
                BoardType: 
                <select name="boardType">
                    <option value="{@boardType}"><xsl:value-of select="@boardType"/></option>
                    <option value="D19C">D19C</option>
                    <option value="CBC3FC7">CBC3FC7</option>
                    <option value="GLIB">GLIB</option>
                    <option value="CTA">CTA</option>
                    <option value="ICGLIB">ICGLIB</option>
                    <option value="ICFC7">ICFC7</option>
                </select>
                <!--<input type="text" size="8" value="{@boardType}">-->
                        <!--<xsl:attribute name="boardType">-->
                            <!--<xsl:value-of select="@boardType"/>-->
                        <!--</xsl:attribute>-->
                <!--</input>-->
            </li>
            <xsl:choose>
                <xsl:when test="@boardType != 'D19C'">
                    <li>
                        EventType: <xsl:value-of select="@eventType"/>
                    </li>
                </xsl:when>
                <xsl:otherwise>
                    <li>
                        EventType:
                        <!--<input type="text" size="8" value="{@eventType}">-->
                                <!--<xsl:attribute name="eventType">-->
                                    <!--<xsl:value-of select="@eventType"/>-->
                                <!--</xsl:attribute>-->
                        <!--</input>-->
                            <input type="radio" name="eventType" value="VR" checked="checked"/>VR
                            <input type="radio" name="eventType" value="ZS"/>ZS
                    </li>
                </xsl:otherwise>
            </xsl:choose>
                  <li>Connection Id:
                  <input type="text" name="connection_id" size="10" value="{connection/@id}">
                          <xsl:attribute name="connection_id">
                              <xsl:value-of select="connection/@id"/>
                          </xsl:attribute>
                      </input>
                  </li>
                  <li>
                  URI: 
                  <input type="text" name="connection_uri" size="28" value="{connection/@uri}">
                          <xsl:attribute name="connection_uri">
                                <xsl:value-of select="connection/@uri"/>
                          </xsl:attribute>
                      </input>
                  </li>
                  <li>
                  Address Table:
                  <input type="text" name="address_table" size="48" value="{connection/@address_table}">
                          <xsl:attribute name="connection_address_table">
                                <xsl:value-of select="connection/@address_table"/>
                          </xsl:attribute>
                      </input>
                  </li>
        </div>
        
            <!--Module-->
            <xsl:apply-templates select="Module"/>
            <!--SLINK-->
            <xsl:apply-templates select="SLink"/>

            <!--Registers-->
            <div class="BeBoardRegisters">
                <h4>Registers</h4>
                <ul>
                <xsl:apply-templates select="Register"/>
            </ul>
            </div>
        </div>
    </xsl:template>

    <!--Template for Module nodes-->
    <xsl:template match="Module">
        <div class="Module">
        <h4> Module Id: <xsl:value-of select="@FeId"/></h4>
            FMCId: <xsl:value-of select="@FMCId"/>
             ModuleId: <xsl:value-of select="@ModuleId"/>
             Status: <input type="text" name="module_status" size="1" value="{@Status}">
                 <xsl:attribute name="Status">
                    <xsl:value-of select="@Status"/>
                </xsl:attribute>
            </input>
            <br></br>
            <div class="CBC">
            CBC FilePath: <input type="text" name="cbc_filepath" size="20" value="{CBC_Files/@path}">
                <xsl:attribute name="Status">
                    <xsl:value-of select="CBC_Files/@path"/>
                </xsl:attribute>
            </input>
            <!--<div class="Config">-->
            <xsl:apply-templates select="Global"/>
            <ul>
            <xsl:apply-templates select="Global_CBC_Register"/>
            </ul>
            </div>
            <ol>
                <xsl:apply-templates select="CBC"/>
            </ol>
            </div>
        <!--</div>-->
    </xsl:template>

    <!--Template for CBC nodes-->
    <xsl:template match="CBC">
        <li> CBC Id: <xsl:value-of select="@Id"/>  ConfigFile: 
            <!--TODO-->
            <select name="configfile{@Id}" onchange='SelectFile(this.value,{@Id});'>
                    <option value="{@configfile}"><xsl:value-of select="@configfile"/></option>
                    <option value="Cbc_default_electron.txt">Cbc_default_electron.txt</option>
                    <option value="Cbc_default_hole.txt">Cbc_default_hole.txt</option>
                    <option value="CBC3_default.txt">CBC3_default.txt</option>
                    <option value="Cbc_default_irrad.txt">Cbc_default_irrad.txt</option>
                    <option value="FE0CBC{@Id}">FE0CBC<xsl:value-of select="@Id"/></option>
                    <option value="other">other</option>
                </select> <br></br><input type="text" name="configfile{@Id}" id="configfile{@Id}" style="display:none;" placeholder="your filename"></input>
                <!--<xsl:attribute name="configfile_text">-->
                    <!--<xsl:value-of select="@configfile"/>-->
                <!--</xsl:attribute>-->
            <!--</input>-->
        </li>
        <!--<div class="Config">-->
        <ul>
            <xsl:apply-templates select="Settings"/>
            <xsl:apply-templates select="TestPulse"/>
            <xsl:apply-templates select="ClusterStub"/>
            <xsl:apply-templates select="Misc"/>
            <xsl:apply-templates select="ChannelMask"/>
            <xsl:if test="Register">
            <li> CBCRegisters: </li>
            <ul>
                <xsl:apply-templates select="Register"/>
            </ul>
            </xsl:if>
        </ul>
        <!--</div>-->
    </xsl:template>

    <!--Template for Global|CBC Settings node-->
    <xsl:template match="Settings">
        <li> Settings:  
            threshold = <input type="text" name="{name(..)}threshold" size="4" value="{@threshold}">
                <xsl:attribute name="threshold">
                    <xsl:value-of select="@threshold"/>
                </xsl:attribute>
            </input>
        latency=<input type="text" name="{name(..)}latency" size="4" value="{@latency}">
                <xsl:attribute name="latency">
                    <xsl:value-of select="@latency"/>
                </xsl:attribute>
            </input>
        </li>
    </xsl:template>

    <!--Template for Global|CBC TestPulse node-->
    <xsl:template match="TestPulse">
        <li> TestPulse: enable
            <!--<input type="text" name="{name(..)}tp_enable" size ="1" value="{@enable}">-->
                <input type="hidden" name="{name(..)}tp_enable" value="0" />
                <input type="checkbox" name="{name(..)}tp_enable" value="1" >
                    <xsl:if test="@enable = 1">
                        <xsl:attribute name="checked">
                        <xsl:value-of select="@checked"/>
                        </xsl:attribute>
                    </xsl:if>
                </input>
            <!--</input>-->
            polarity=
            <input type="radio" name="{name(..)}tp_polarity" value="0">
                    <xsl:if test="@polarity = 0">
                        <xsl:attribute name="checked">
                        <xsl:value-of select="@checked"/>
                        </xsl:attribute>
                    </xsl:if>
                </input>electron
                <input type="radio" name="{name(..)}tp_polarity" value="1">
                    <xsl:if test="@polarity = 1">
                        <xsl:attribute name="checked">
                        <xsl:value-of select="@checked"/>
                        </xsl:attribute>
                    </xsl:if>
                </input>hole
            <!--<input type="text" name="{name(..)}tp_polarity" size ="1" value="{@polarity}">-->
               <!--<xsl:attribute name="polarity">-->
                   <!--<xsl:value-of select="@polarity"/>-->
               <!--</xsl:attribute>-->
           <!--</input>-->
            amplitude=
            <input type="text" name="{name(..)}tp_amplitude" size ="5" value="{@amplitude}">
               <xsl:attribute name="amplitude">
                   <xsl:value-of select="@amplitude"/>
               </xsl:attribute>
           </input>
            channelgroup=
            <!--<input type="text" name="{name(..)}tp_channelgroup" size ="5" value="{@channelgroup}">-->
               <!--<xsl:attribute name="channelgroup">-->
                   <!--<xsl:value-of select="@channelgroup"/>-->
               <!--</xsl:attribute>-->
           <!--</input>-->
            <input type="number" name="{name(..)}tp_channelgroup" size="5" min="0" max="7" value="{@channelgroup}"/>
            delay=
            <input type="number" name="{name(..)}tp_delay" size="5" min="0" max="24" value="{@delay}"/>
            <!--<input type="text" name="{name(..)}tp_delay" size ="5" value="{@delay}">-->
               <!--<xsl:attribute name="delay">-->
                   <!--<xsl:value-of select="@delay"/>-->
               <!--</xsl:attribute>-->
           <!--</input>-->
            groundOthers=
            <input type="hidden" name="{name(..)}tp_groundothers" value="0" />
                <input type="checkbox" name="{name(..)}tp_groundothers" value="1" >
                    <xsl:if test="@groundothers = 1">
                        <xsl:attribute name="checked">
                        <xsl:value-of select="@checked"/>
                        </xsl:attribute>
                    </xsl:if>
                </input>
            <!--<input type="text" name="{name(..)}tp_groundothers" size ="1" value="{@groundothers}">-->
               <!--<xsl:attribute name="groundothers">-->
                   <!--<xsl:value-of select="@groundothers"/>-->
               <!--</xsl:attribute>-->
           <!--</input>-->
        </li> 
    </xsl:template>

    <!--Template for Global|CBC ClusterStub node-->
    <xsl:template match="ClusterStub">
        <li> ClusterStub: clusterwidth=
            <input type="text" name="{name(..)}cs_clusterwidth" size ="1" value="{@clusterwidth}">
                <xsl:attribute name="clusterwidth">
                    <xsl:value-of select="@clusterwidth"/>
                </xsl:attribute>
            </input>
            ptwidth=
            <input type="text" name="{name(..)}cs_ptwidth" size ="1" value="{@ptwidth}">
               <xsl:attribute name="ptwidth">
                   <xsl:value-of select="@ptwidth"/>
               </xsl:attribute>
           </input>
            layerswap=
                <input type="hidden" name="{name(..)}cs_layerswap" value="0" />
                <input type="checkbox" name="{name(..)}cs_layerswap" value="1" >
                    <xsl:if test="@layerswap = 1">
                        <xsl:attribute name="checked">
                        <xsl:value-of select="@checked"/>
                        </xsl:attribute>
                    </xsl:if>
                </input>
            <!--<input type="text" name="{name(..)}cs_layerswap" size ="1" value="{@layerswap}">-->
               <!--<xsl:attribute name="layerswap">-->
                   <!--<xsl:value-of select="@layerswap"/>-->
               <!--</xsl:attribute>-->
           <!--</input>-->
            off1=
            <input type="text" name="{name(..)}cs_off1" size ="2" value="{@off1}">
               <xsl:attribute name="off1">
                   <xsl:value-of select="@off1"/>
               </xsl:attribute>
           </input>
            off2=
            <input type="text" name="{name(..)}cs_off2" size ="2" value="{@off2}">
               <xsl:attribute name="off2">
                   <xsl:value-of select="@off2"/>
               </xsl:attribute>
           </input>
            off3=
            <input type="text" name="{name(..)}cs_off3" size ="2" value="{@off3}">
               <xsl:attribute name="off3">
                   <xsl:value-of select="@off3"/>
               </xsl:attribute>
           </input>
            off4=
            <input type="text" name="{name(..)}cs_off4" size ="2" value="{@off4}">
               <xsl:attribute name="off4">
                   <xsl:value-of select="@off4"/>
               </xsl:attribute>
           </input>
        </li> 
    </xsl:template>

    <!--Template for Global|CBC Misc node-->
    <xsl:template match="Misc">
        <li> Misc: analogmux=
            <input type="text" name="{name(..)}misc_amux" size ="8" value="{@analogmux}">
                <xsl:attribute name="analogmux">
                    <xsl:value-of select="@analogmux"/>
                </xsl:attribute>
            </input>
            pipelogic=
            <input type="text" name="{name(..)}misc_piplogic" size ="1" value="{@pipelogic}">
               <xsl:attribute name="pipelogic">
                   <xsl:value-of select="@pipelogic"/>
               </xsl:attribute>
           </input>
            stublogic=
            <input type="text" name="{name(..)}misc_stublogic" size ="1" value="{@stublogic}">
               <xsl:attribute name="stublogic">
                   <xsl:value-of select="@stublogic"/>
               </xsl:attribute>
           </input>
            or254=
                <input type="hidden" name="{name(..)}misc_or254" value="0" />
                <input type="checkbox" name="{name(..)}misc_or254" value="1" >
                    <xsl:if test="@or254 = 1">
                        <xsl:attribute name="checked">
                        <xsl:value-of select="@checked"/>
                        </xsl:attribute>
                    </xsl:if>
                </input>
            <!--<input type="text" name="{name(..)}misc_or254" size ="2" value="{@or254}">-->
               <!--<xsl:attribute name="or254">-->
                   <!--<xsl:value-of select="@or254"/>-->
               <!--</xsl:attribute>-->
           <!--</input>-->
            tpgclock=
                <input type="hidden" name="{name(..)}misc_tpgclock" value="0" />
                <input type="checkbox" name="{name(..)}misc_tpgclock" value="1" >
                    <xsl:if test="@tpgclock = 1">
                        <xsl:attribute name="checked">
                        <xsl:value-of select="@checked"/>
                        </xsl:attribute>
                    </xsl:if>
                </input>
            <!--<input type="text" name="{name(..)}misc_tpgclock" size ="2" value="{@tpgclock}">-->
               <!--<xsl:attribute name="tpgclock">-->
                   <!--<xsl:value-of select="@tpgclock"/>-->
               <!--</xsl:attribute>-->
           <!--</input>-->
            testclock=
                <input type="hidden" name="{name(..)}misc_testclock" value="0" />
                <input type="checkbox" name="{name(..)}misc_testclock" value="1" >
                    <xsl:if test="@testclock = 1">
                        <xsl:attribute name="checked">
                        <xsl:value-of select="@checked"/>
                        </xsl:attribute>
                    </xsl:if>
                </input>
            <!--<input type="text" name="{name(..)}misc_testclock" size ="2" value="{@testclock}">-->
               <!--<xsl:attribute name="testclock">-->
                   <!--<xsl:value-of select="@testclock"/>-->
               <!--</xsl:attribute>-->
           <!--</input>-->
            dll=
            <input type="text" name="{name(..)}misc_dll" size ="2" value="{@dll}">
               <xsl:attribute name="dll">
                   <xsl:value-of select="@dll"/>
               </xsl:attribute>
           </input>
        </li> 
    </xsl:template>

    <!--Template for Global|CBC ChannelMask node-->
    <xsl:template match="ChannelMask">
        <li> ChannelMask (comma separated list):
            <input type="text" name="{name(..)}chanmas_disable" size ="18" value="{@disable}">
                <xsl:attribute name="disable">
                    <xsl:value-of select="@disable"/>
                </xsl:attribute>
            </input>
        </li>
    </xsl:template>

    <!--Template for GlobalNodes-->
    <xsl:template match="Global">
        <h5> Global CBC Config </h5>
        <div class="Config">
            <ul>
                <xsl:apply-templates select="Settings"/>
                <xsl:apply-templates select="TestPulse"/>
                <xsl:apply-templates select="ClusterStub"/>
                <xsl:apply-templates select="Misc"/>
                <xsl:apply-templates select="ChannelMask"/>
            </ul>
        </div>
    </xsl:template>

    <!--Template for GlobalCBCRegister node-->
    <xsl:template match="Global_CBC_Register">
        <li> GlobalCBCRegister: <xsl:value-of select="@name"/>
            <input type="text" name="glob_cbc_reg" size ="5" value="{.}">
                <xsl:attribute name="Global_CBC_Register">
                    <xsl:value-of select="Global_CBC_Register"/>
                </xsl:attribute>
            </input>
        </li>
    </xsl:template>


    <!--Template for SLINK node-->
    <xsl:template match="SLink">
    <div class="SLink">
    <h4>SLink</h4>
        <ul>
            <xsl:apply-templates select="DebugMode"/>
            <xsl:apply-templates select="ConditionData"/>
        </ul>
    </div>
    </xsl:template>

    <!--Template for SLink Debug Mode node-->
    <xsl:template match="DebugMode">
        <li>
             Debug Mode:
             <!--<input type="text" name="slink_debugmode" size ="7" value="{@type}">-->
                 <!--<xsl:attribute name="type">-->
                     <!--<xsl:value-of select="@type"/>-->
                 <!--</xsl:attribute>-->
             <!--</input>-->
            <select name="debugMode">
                <option name="{@type}"><xsl:value-of select="@type"/></option>
                <option name="FULL">FULL</option>
                <option name="SUMMARY">SUMMARY</option>
                <option name="ERROR">ERROR</option>
            </select>
       </li>
   </xsl:template>

    <!--Template for SLINKConditionData node-->
    <xsl:template match="ConditionData">
               <li>
                   Type:
                       <select name="ConditionData" id="conddata_{position()}" onchange='DisplayFields(this.value, {position()});'>
                        <option name="{@type}"><xsl:value-of select="@type"/></option>
                        <option name="I2C">I2C</option>
                        <option name="User">User</option>
                        <option name="HV">HV</option>
                        <option name="TDC">TDC</option>
                   </select>
                        
                   <label for="conddata_Register_{position()}" id="conddata_RegisterLabel_{position()}" style="display:none">Register:</label>
                   <input type="text" name="ConditionData_Register" id="conddata_Register_{position()}" value="{@Register}" size="10" style="display:none"/>

                   <label for="conddata_UID_{position()}" id="conddata_UIDLabel_{position()}" style="display:none">UID:</label>
                   <input type="text" name="ConditionData_UID" id="conddata_UID_{position()}" value="{@UID}" size="5" style="display:none"/>

                   <label for="conddata_FeId_{position()}" id="conddata_FeIdLabel_{position()}" style="display:none">FeId:</label>
                   <input type="text" name="ConditionData_FeId" id="conddata_FeId_{position()}" value="{@FeId}" size="5" style="display:none"/>

                   <label for="conddata_CbcId_{position()}" id="conddata_CbcIdLabel_{position()}" style="display:none">CbcId:</label>
                   <input type="text" name="ConditionData_CbcId" id="conddata_CbcId_{position()}" value="{@CbcId}" size="5" style="display:none"/> 
                   
                   <label for="conddata_Sensor_{position()}" id="conddata_SensorLabel_{position()}" style="display:none">Sensor:</label>
                   <input type="text" name="ConditionData_Sensor" id="conddata_Sensor_{position()}" value="{@Sensor}" size="5" style="display:none"/>

                   <label for="conddata_Value_{position()}" id="conddata_ValueLabel_{position()}" style="display:none">Value:</label>
                   <input type="text" name="ConditionData_Value" id="conddata_Value_{position()}" value="{.}" size="5" style="display:none"/>
               </li>

   </xsl:template>
    

    <!--Template for RegisterNode with children-->
    <xsl:template match="Register[Register]">
        <li><xsl:value-of select="@name"/></li>
            <ul>
                <xsl:apply-templates select="Register"/>
            </ul>
    </xsl:template>

    <!--Template for Register Node without children-->
    <xsl:template match="Register[not(Register)]">
        <li> <xsl:value-of select="@name"/>:
            <!--<xsl:value-of select="@name"/>-->
            <input type="text" name="{../../../@name}.{../../@name}.{../@name}.{@name}" size ="5" value="{.}">
                <!--<xsl:attribute name="Register">-->
                    <!--<xsl:value-of select="Register"/>-->
                <!--</xsl:attribute>-->
            </input>
        </li>
    </xsl:template>

</xsl:stylesheet>
