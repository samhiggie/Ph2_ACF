<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

    <xsl:template match="/*">
        <html>
            <body>
                <h2>HWDescription</h2>
                   <div>
                     <xsl:for-each select="BeBoard">
                        <div>
                           <table border="0">
                             <tr>
                                 <h3>BeBoard</h3>
                                <td>Id: <xsl:value-of select="@Id"/></td>
                                <td>BoardType: <xsl:value-of select="@boardType"/></td>
                                <td>EventType: <xsl:value-of select="@eventType"/></td>
                             </tr>
                           </table>
                            <table border="0">
                              <tr>
                                <td>ConnectionId: <xsl:value-of select="connection/@id"/></td>
                                <td>URI: <xsl:value-of select="connection/@uri"/></td>
                                <td>Address Table: <xsl:value-of select="connection/@address_table"/></td>
                              </tr>
                            </table>
                            
                            <xsl:for-each select="Module">
                                <div>
                                <h3>Module</h3>
                                <table border="0">
                                  <tr>
                                    <td>FeId: <xsl:value-of select="@FeId"/></td>
                                    <td>FMCId: <xsl:value-of select="@FMCId"/></td>
                                    <td>ModuleId: <xsl:value-of select="@ModuleId"/></td>
                                    <td>Status: <xsl:value-of select="@Status"/></td>
                                    <td>CBC FilePath: <xsl:value-of select="CBC_Files/@path"/></td>
                                  </tr>
                                </table>

                                <xsl:for-each select="CBC">
                                    <div>
                                        <table>
                                            <tr>
                                                <td>Cbc Id: <xsl:value-of select="@Id"/></td><td> ConfigFile: <xsl:value-of select="@configfile"/></td>    
                                            </tr>
                                        </table>
                                   <!--[>CBC<]-->
                                   </div>
                               </xsl:for-each>
                            <!--[>Module<]-->
                            </div>
                            </xsl:for-each>

                            <!--SLINK-->
                            <div>
                                <h3>SLINK</h3>
                                Debug Mode: <xsl:value-of select="SLink/DebugMode/@type"/>
                                <h4>ConditionData</h4>
                                <xsl:for-each select="SLink/ConditionData">
                                    <table border="0">
                                        <tr>
                                            <td>Type: <xsl:value-of select="@type"/></td>
                                            <xsl:if test="@type = 'I2C'">
                                            <td>Register: <xsl:value-of select="@Register"/></td>
                                            <td>FeId: <xsl:value-of select="@FeId"/></td>
                                            <td>CbcId: <xsl:value-of select="@CbcId"/></td>
                                            </xsl:if>
                                            <xsl:if test="@type = 'User'">
                                            <td>UID: <xsl:value-of select="@UID"/></td>
                                            <td>FeId: <xsl:value-of select="@FeId"/></td>
                                            <td>CbcId: <xsl:value-of select="@CbcId"/></td>
                                            <td>Value: <xsl:value-of select="."/></td>
                                            </xsl:if>
                                            <xsl:if test="@type = 'HV'">
                                            <td>FeId: <xsl:value-of select="@FeId"/></td>
                                            <td>Sensor: <xsl:value-of select="@Sensor"/></td>
                                            <td>Value: <xsl:value-of select="."/></td>
                                            </xsl:if>
                                            <xsl:if test="@type = 'TDC'">
                                            <td>FeId: <xsl:value-of select="@FeId"/></td>
                                            </xsl:if>
                                        </tr>
                                    </table>
                                </xsl:for-each>
                            </div>
                            <!--SLINK-->

                            <!--Registers-->
                            <div>
                                <h4>Registers</h4>
                                <xsl:for-each select="Register">
                                    <table border="0">
                                        <tr>
                                            <!--if no content-->
                                            <xsl:choose>
                                            <xsl:when test="*">
                                                 <xsl:for-each select="Register">
                                                      <xsl:choose>
                                                      <xsl:when test="*">
                                                         <xsl:for-each select="Register">
                                                            <xsl:choose>
                                                            <xsl:when test="*">
                                                               <xsl:for-each select="Register">
                                                                    <xsl:choose>
                                                                    <xsl:when test="*">
                                                                       <xsl:for-each select="Register">
                                                                                   <td>Register: <xsl:value-of select="@name"/> </td>
                                                                       </xsl:for-each>
                                                                     </xsl:when>
                                                                     <xsl:otherwise>
                                                                         <td>Register: <xsl:value-of select="@name"/> - <xsl:value-of select="."/> </td>
                                                                     </xsl:otherwise>
                                                                    </xsl:choose>
                                                               </xsl:for-each>
                                                             </xsl:when>
                                                             <xsl:otherwise>
                                                                 <td>Register: <xsl:value-of select="@name"/> - <xsl:value-of select="."/> </td>
                                                             </xsl:otherwise>
                                                            </xsl:choose>
                                                         </xsl:for-each>
                                                       </xsl:when>
                                                       <xsl:otherwise>
                                                           <td>Register: <xsl:value-of select="@name"/> - <xsl:value-of select="."/> </td>
                                                       </xsl:otherwise>
                                                      </xsl:choose>
                                                 </xsl:for-each>
                                            </xsl:when>
                                            <xsl:otherwise>
                                                <td>Register: <xsl:value-of select="@name"/> - <xsl:value-of select="."/> </td>
                                            </xsl:otherwise>
                                        </xsl:choose>
                                        </tr>
                                    </table>
                                </xsl:for-each>
                            </div>
                            <!--Registers-->
                    <!--BeBoard-->
                    </div>
                    </xsl:for-each>
                 <!--HWDescription-->
                </div>
            </body>
        </html>
    </xsl:template>
</xsl:stylesheet>
