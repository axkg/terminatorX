<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:template match="section">
    <html>
      <head>
        <meta content="Alexander Koenig" name="Author"/>
        <meta content="text/html; charset=iso-8859-1" http-equiv="Content-Type"/>
        <link href="favicon.ico" rel="icon" type="image/x-icon"/>
        <link href="favicon.ico" rel="shortcut icon" type="image/x-icon"/>
        <title>terminatorX: <xsl:value-of select="@name"/>
        </title>
        <script language="Javascript" src="script.js" type="text/javascript"/>
        <style type="text/css"> A:hover {text-decoration: none; color: #ff4444} A:active
          {text-decoration: none; color: #ff4444} body { font-family: vera-sans, Verdana, sans-serif; } p.plain
          { text-align: left } p.fancy { text-align: justify } </style>
      </head>
      <body alink="#FF0000" bgcolor="#555555" link="#FFFF99"
        onload="rolloverLoad('main','pix/vinyl-hover.gif','pix/vinyl.png');                rolloverLoad('download','pix/vinyl-hover.gif','pix/vinyl.png');         rolloverLoad('screenshots','pix/vinyl-hover.gif','pix/vinyl.png');         rolloverLoad('faq','pix/vinyl-hover.gif','pix/vinyl.png');         rolloverLoad('docs','pix/vinyl-hover.gif','pix/vinyl.png');         rolloverLoad('turntable','pix/vinyl-hover.gif','pix/vinyl.png');         rolloverLoad('scratches','pix/vinyl-hover.gif','pix/vinyl.png');         rolloverLoad('links','pix/vinyl-hover.gif','pix/vinyl.png');        rolloverLoad('bugs','pix/vinyl-hover.gif','pix/vinyl.png');      rolloverLoad('aseqjoy','pix/vinyl-hover.gif','pix/vinyl.png');"
        text="#FFFFFF" vlink="#FFCC33">
        <!--new-->
        <table border="0" cellpadding="0" cellspacing="0" width="100%">
          <tr>
            <td bgcolor="#000000" colspan="2">
              <table border="0" cellspacing="5px" width="100%">
                <tr>
                  <td>
                    <center>
                      <img ALT="terminatorX" src="pix/tX_logo.jpg"/>
                    </center>
                  </td>
                </tr>
              </table>
            </td>
          </tr>
          <tr>
            <td align="left" bgcolor="#ffffff">
              <font color="#ffffff" size="+2"> menu</font>
            </td>
            <td bgcolor="#ffffff" width="100%">
              <table border="0" cellpadding="4px" cellspacing="0" width="100%">
                <tr>
                  <td align="right" bgcolor="#ffffff" width="100%">
                    <font color="#000000" size="+2">
                      <b>
                        <xsl:value-of select="@name"/>
                      </b>
                    </font>
                  </td>
                </tr>
              </table>
            </td>
          </tr>
          <tr>
            <!--menu-->
            <td valign="top">
              <table border="0" cellpadding="2px" cellspacing="0">
                <tr>
                  <xsl:if test="@name='main'">
                    <td bgcolor="#777777">
                      <img alt="-" border="0" src="pix/vinyl-highlight.png" vspace="0"/>
                    </td>
                    <td bgcolor="#777777">main</td>
                    <td bgcolor="#777777">
                      <img alt="-" border="0" src="pix/vinyl-spacer.png" vspace="0"/>
                    </td>
                  </xsl:if>
                  <xsl:if test="@name!='main'">
                    <td>
                      <img alt="-" border="0" name="main" src="pix/vinyl.png" vspace="0"/>
                    </td>
                    <td>
                      <a href="./" onMouseOut="rolloverOff('main');" onMouseOver="rolloverOn('main');">main</a>
                    </td>
                    <td/>
                  </xsl:if>
                </tr>
                <tr>
                  <xsl:if test="@name='download'">
                    <td bgcolor="#777777">
                      <img alt="-" border="0" src="pix/vinyl-highlight.png" vspace="0"/>
                    </td>
                    <td bgcolor="#777777">download</td>
                    <td bgcolor="#777777">
                      <img alt="-" border="0" src="pix/vinyl-spacer.png" vspace="0"/>
                    </td>
                  </xsl:if>
                  <xsl:if test="@name!='download'">
                    <td>
                      <img alt="-" border="0" name="download" src="pix/vinyl.png" vspace="0"/>
                    </td>
                    <td>
                      <a href="download.html" onMouseOut="rolloverOff('download');" onMouseOver="rolloverOn('download');">download</a>
                    </td>
                    <td/>
                  </xsl:if>
                </tr>
                <tr>
                  <xsl:if test="@name='screenshots'">
                    <td bgcolor="#777777">
                      <img alt="-" border="0" src="pix/vinyl-highlight.png" vspace="0"/>
                    </td>
                    <td bgcolor="#777777">screenshots</td>
                    <td bgcolor="#777777">
                      <img alt="-" border="0" src="pix/vinyl-spacer.png" vspace="0"/>
                    </td>
                  </xsl:if>
                  <xsl:if test="@name!='screenshots'">
                    <td>
                      <img alt="-" border="0" name="screenshots" src="pix/vinyl.png" vspace="0"/>
                    </td>
                    <td>
                      <a href="screenshots.html" onMouseOut="rolloverOff('screenshots');" onMouseOver="rolloverOn('screenshots');">screenshots</a>
                    </td>
                    <td/>
                  </xsl:if>
                </tr>
                <tr>
                  <xsl:if test="@name='bugs'">
                    <td bgcolor="#777777">
                      <img alt="-" border="0" src="pix/vinyl-highlight.png" vspace="0"/>
                    </td>
                    <td bgcolor="#777777">bugs</td>
                    <td bgcolor="#777777">
                      <img alt="-" border="0" src="pix/vinyl-spacer.png" vspace="0"/>
                    </td>
                  </xsl:if>
                  <xsl:if test="@name!='bugs'">
                    <td>
                      <img alt="-" border="0" name="bugs" src="pix/vinyl.png" vspace="0"/>
                    </td>
                    <td>
                      <a href="bugs.html" onMouseOut="rolloverOff('bugs');" onMouseOver="rolloverOn('bugs');">bugs</a>
                    </td>
                    <td/>
                  </xsl:if>
                </tr>                
                <tr>
                  <xsl:if test="@name='faq'">
                    <td bgcolor="#777777">
                      <img alt="-" border="0" src="pix/vinyl-highlight.png" vspace="0"/>
                    </td>
                    <td bgcolor="#777777">faq</td>
                    <td bgcolor="#777777">
                      <img alt="-" border="0" src="pix/vinyl-spacer.png" vspace="0"/>
                    </td>
                  </xsl:if>
                  <xsl:if test="@name!='faq'">
                    <td>
                      <img alt="-" border="0" name="faq" src="pix/vinyl.png" vspace="0"/>
                    </td>
                    <td>
                      <a href="faq.html" onMouseOut="rolloverOff('faq');" onMouseOver="rolloverOn('faq');">faq</a>
                    </td>
                    <td/>
                  </xsl:if>
                </tr>                 
                <tr>
                  <xsl:if test="@name='docs'">
                    <td bgcolor="#777777">
                      <img alt="-" border="0" src="pix/vinyl-highlight.png" vspace="0"/>
                    </td>
                    <td bgcolor="#777777">docs</td>
                    <td bgcolor="#777777">
                      <img alt="-" border="0" src="pix/vinyl-spacer.png" vspace="0"/>
                    </td>
                  </xsl:if>
                  <xsl:if test="@name!='docs'">
                    <td>
                      <img alt="-" border="0" name="docs" src="pix/vinyl.png" vspace="0"/>
                    </td>
                    <td>
                      <a href="docs.html" onMouseOut="rolloverOff('docs');"
                        onMouseOver="rolloverOn('docs');"> docs</a>
                    </td>
                    <td/>
                  </xsl:if>
                </tr>
                <tr>
                  <xsl:if test="@name='scratches'">
                    <td bgcolor="#777777">
                      <img alt="-" border="0" src="pix/vinyl-highlight.png" vspace="0"/>
                    </td>
                    <td bgcolor="#777777">scratches</td>
                    <td bgcolor="#777777">
                      <img alt="-" border="0" src="pix/vinyl-spacer.png" vspace="0"/>
                    </td>
                  </xsl:if>
                  <xsl:if test="@name!='scratches'">
                    <td>
                      <img alt="-" border="0" name="scratches" src="pix/vinyl.png" vspace="0"/>
                    </td>
                    <td>
                      <a href="scratches.html" onMouseOut="rolloverOff('scratches');" onMouseOver="rolloverOn('scratches');">scratches</a>
                    </td>
                    <td/>
                  </xsl:if>
                </tr>
                <tr>
                  <xsl:if test="@name='turntable gallery'">
                    <td bgcolor="#777777">
                      <img alt="-" border="0" src="pix/vinyl-highlight.png" vspace="0"/>
                    </td>
                    <td bgcolor="#777777">turntables</td>
                    <td bgcolor="#777777">
                      <img alt="-" border="0" src="pix/vinyl-spacer.png" vspace="0"/>
                    </td>
                  </xsl:if>
                  <xsl:if test="@name!='turntable gallery'">
                    <td>
                      <img alt="-" border="0" name="turntable" src="pix/vinyl.png" vspace="0"/>
                    </td>
                    <td>
                      <a href="turntable.html" onMouseOut="rolloverOff('turntable');" onMouseOver="rolloverOn('turntable');">turntables</a>
                    </td>
                    <td/>
                  </xsl:if>
                </tr>
                <tr>
                  <xsl:if test="@name='aseqjoy'">
                    <td bgcolor="#777777">
                      <img alt="-" border="0" src="pix/vinyl-highlight.png" vspace="0"/>
                    </td>
                    <td bgcolor="#777777">aseqjoy</td>
                    <td bgcolor="#777777">
                      <img alt="-" border="0" src="pix/vinyl-spacer.png" vspace="0"/>
                    </td>
                  </xsl:if>
                  <xsl:if test="@name!='aseqjoy'">
                    <td>
                      <img alt="-" border="0" name="aseqjoy" src="pix/vinyl.png" vspace="0"/>
                    </td>
                    <td>
                      <a href="aseqjoy.html" onMouseOut="rolloverOff('aseqjoy');"
                        onMouseOver="rolloverOn('aseqjoy');"> aseqjoy</a>
                    </td>
                    <td/>
                  </xsl:if>
                </tr>
                <tr>
                  <xsl:if test="@name='links'">
                    <td bgcolor="#777777">
                      <img alt="-" border="0" src="pix/vinyl-highlight.png" vspace="0"/>
                    </td>
                    <td bgcolor="#777777">links</td>
                    <td bgcolor="#777777">
                      <img alt="-" border="0" src="pix/vinyl-spacer.png" vspace="0"/>
                    </td>
                  </xsl:if>
                  <xsl:if test="@name!='links'">
                    <td>
                      <img alt="-" border="0" name="links" src="pix/vinyl.png" vspace="0"/>
                    </td>
                    <td>
                      <a href="links.html" onMouseOut="rolloverOff('links');"
                        onMouseOver="rolloverOn('links');"> links</a>
                    </td>
                    <td/>
                  </xsl:if>
                </tr>
              </table>
            </td>
            <!--contents-->
            <td valign="top">
              <table border="0" cellpadding="10px" cellspacing="0" width="100%">
                <tr>
                  <td bgcolor="#777777" width="100%">
                    <xsl:apply-templates/>
                  </td>
                </tr>
              </table>
            </td>
          </tr>
        </table>
        <!-- bottom -->
        <HR width="100%"/>
        <table width="100%">
          <tr>
            <td align="left" valign="top" width="100%">
              <font size="-1">Copyright (C) 1999-2003 by <A HREF="mailto:alex@lisas.de">Alexander KMAKE_THIS_MYOUMLnig</A>
              </font>
            </td>
            <td>
              <a href="http://validator.w3.org/check/referer">
                <img alt="Valid HTML 4.01!" border="0" height="31" src="pix/valid-html401.png" width="88"/>
              </a>
            </td>
          </tr>
        </table>
      </body>
    </html>
  </xsl:template>
  <xsl:template match="heading">
    <font size="+2">
      <xsl:apply-templates/>
    </font>
    <br/>
  </xsl:template>
  <xsl:template match="subtitle">
    <font size="-2">
      <xsl:apply-templates/>
    </font>
  </xsl:template>
  <xsl:template match="para">
    <xsl:if test="@class='fancy'">
      <p class="fancy">
        <xsl:apply-templates/>
      </p>
    </xsl:if>
    <xsl:if test="@class='plain'">
      <p class="plain">
        <xsl:apply-templates/>
      </p>
    </xsl:if>
  </xsl:template>
  <xsl:template match="italic">
    <i>
      <xsl:apply-templates/>
    </i>
  </xsl:template>
  <xsl:template match="red">
    <font color="#FF4444">
      <xsl:apply-templates/>
    </font>
  </xsl:template>
  <xsl:template match="bold">
    <b>
      <xsl:apply-templates/>
    </b>
  </xsl:template>
  <xsl:template match="newsheader"/>
  <xsl:template match="newsitem">
    <table border="0" cellpadding="0" cellspacing="0">
      <!-- header -->
      <tr>
        <td bgcolor="#999999">
          <xsl:for-each select="newsheader">
            <table border="0" cellpadding="2px" cellspacing="0">
              <tr>
                <td>
                  <img alt="new:" src="pix/new.png"/>
                </td>
                <td align="left" valign="middle" width="100%">
                  <font size="+1">
                    <xsl:apply-templates/>
                  </font>
                  <font color="#DDDDDD" size="+1"> [<xsl:value-of select="@date"/>]</font>
                </td>
              </tr>
            </table>
          </xsl:for-each>
        </td>
      </tr>
      <!-- contents -->
      <tr>
        <td>
          <xsl:apply-templates/>
        </td>
      </tr>
    </table>
  </xsl:template>
  <xsl:template match="link">
    <a>
      <xsl:attribute name="href">
        <xsl:value-of select="@ref"/>
      </xsl:attribute>
      <xsl:apply-templates/>
    </a>
  </xsl:template>
  <xsl:template match="dblink">
    <xsl:variable name="search" select="@id"/>
    <a>
      <xsl:attribute name="href">
        <xsl:value-of select="//dlink[@id=$search]/@url"/>
      </xsl:attribute>
      <xsl:choose>
        <xsl:when test="./text()">
          <xsl:apply-templates/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="//dlink[@id=$search]/@text"/>
        </xsl:otherwise>
      </xsl:choose>
    </a>
    <xsl:if test="sum(//dlink[@id=$search])=0">
      <font color="#FF0000">
        <blink>broken dblink &quot;<xsl:value-of select="$search"/>&quot;</blink>
      </font>
    </xsl:if>
  </xsl:template>
  <xsl:template match="newline">
    <br/>
  </xsl:template>
  <xsl:template match="itemizedlist">CLOSE_DA_P <ul>
      <xsl:apply-templates/>
    </ul>OPEN_DA_P</xsl:template>
  <xsl:template match="listitem">
    <li>
      <xsl:apply-templates/>
    </li>
  </xsl:template>
  <xsl:template match="person">
    <a>
      <xsl:attribute name="href">mailto:<xsl:if test="email='alex@lisas.de'">alex@lisas.de</xsl:if>
        <xsl:if test="email!='alex@lisas.de'">NOSPAM_<xsl:value-of
            select="substring-before(email, '@')"/>_AT_<xsl:value-of select="substring-after(email, '@')"/>_NOSPAM</xsl:if>
      </xsl:attribute>
      <xsl:value-of select="name"/>
    </a>
  </xsl:template>
  <xsl:template match="index">
    <p>
      <font size="+2">
        <b><xsl:value-of select="title"/></b>
      </font>
    </p>
    <ul>
      <xsl:for-each select="//subsection">
        <li>
          <a>
            <xsl:attribute name="href">#<xsl:value-of select="title"/>
            </xsl:attribute>
            <xsl:value-of select="title"/>
          </a>
        </li>
      </xsl:for-each>
    </ul>
    <p/>
  </xsl:template>
  <xsl:template match="subsection">
    <p>
      <font size="+1"><b>
        <a>
          <xsl:attribute name="name">
            <xsl:value-of select="title"/>
          </xsl:attribute>
          <xsl:value-of select="title"/>
        </a>
      </b></font>
    </p>
    <xsl:apply-templates/>
  </xsl:template>
  <xsl:template match="title"/>
  <xsl:template match="tarfiles">
    <ul>
      <xsl:for-each select="tar">
        <xsl:if test="@version!=//current/@version">
          <li>Version <xsl:value-of select="@version"/>: <a>
              <xsl:attribute name="href">dist/terminatorX-<xsl:value-of
                select="@version"/>.tar.gz</xsl:attribute>terminatorX-<xsl:value-of select="@version"/>.tar.gz</a>
            <xsl:if test="@havebz2='yes'"> (<a>
                <xsl:attribute name="href">dist/terminatorX-<xsl:value-of select="@version"/>.tar.bz2</xsl:attribute>bz2</a>)</xsl:if>
          </li>
        </xsl:if>
      </xsl:for-each>
    </ul>
  </xsl:template>
  <xsl:template match="rpm" name="rpm">
    <li>
      <xsl:if test="@type='src'">Source RPM: </xsl:if>
      <xsl:if test="@type!='src'">
        <xsl:value-of select="@type"/>-binary RPM: </xsl:if>
      <a>
        <xsl:attribute name="href">rpms/terminatorX-<xsl:value-of select="@version"/>-<xsl:value-of
            select="@rpmsubversion"/>.<xsl:value-of select="@type"/>.<xsl:if test="@ext">
            <xsl:value-of select="@ext"/>.</xsl:if>rpm</xsl:attribute> terminatorX-<xsl:value-of
          select="@version"/>-<xsl:value-of select="@rpmsubversion"/>.<xsl:value-of
          select="@type"/>.<xsl:if test="@ext">
          <xsl:value-of select="@ext"/>.</xsl:if>rpm</a>
      <xsl:if test="@distribution"> built for: <i>
          <xsl:value-of select="@distribution"/>
        </i>
      </xsl:if>
      <xsl:if test="@note">
        <b> note: </b>
        <xsl:value-of select="@note"/>
      </xsl:if>
    </li>
  </xsl:template>
  <xsl:template match="rpmfiles">
    <ul>
      <xsl:for-each select="rpm[@version!=//current/@version]">
        <xsl:call-template name="rpm"/>
      </xsl:for-each>
    </ul>
  </xsl:template>
  <xsl:template match="currentversion">
    <p>The current terminatorX release is Version <xsl:value-of select="//current/@version"/>.
      <br/>Download as:</p>
    <ul>
      <li>tar file: <a>
          <xsl:attribute name="href">dist/terminatorX-<xsl:value-of
            select="//current/@version"/>.tar.gz</xsl:attribute>terminatorX-<xsl:value-of select="//current/@version"/>.tar.gz</a>
        <xsl:if test="//current/@havebz2='yes'"> (<a>
            <xsl:attribute name="href">dist/terminatorX-<xsl:value-of select="//current/@version"/>.tar.bz2</xsl:attribute>bz2</a>)</xsl:if>
      </li>
      <xsl:for-each select="//rpm[@version=//current/@version]">
        <xsl:call-template name="rpm"/>
      </xsl:for-each>
    </ul>
    <xsl:if test="sum(//rpm[@version=//current/@version])=0">Sorry, no RPMs available for the
      current release, yet. Stay tuned.</xsl:if>
  </xsl:template>
  <xsl:template match="filelist">
    <ul>
      <xsl:for-each select="file">
        <li>
          <xsl:call-template name="file"/>
        </li>
      </xsl:for-each>
    </ul>
  </xsl:template>
  <xsl:template match="file" name="file">
    <a>
      <xsl:attribute name="href">files/<xsl:apply-templates/>
      </xsl:attribute>
      <xsl:apply-templates/>
    </a>
  </xsl:template>
  <xsl:template match="faq">
    <a name="index">
      <font size="+1">Question-Index:</font>
    </a>
    <ul>
      <xsl:for-each select="qa">
        <li>
          <a>
            <xsl:attribute name="href">#<xsl:value-of select="position()"/>
            </xsl:attribute>
            <xsl:value-of select="question"/>
          </a>
        </li>
      </xsl:for-each>
    </ul>
    <p>
      <font size="+1">Answers:</font>
    </p>
    <xsl:apply-templates/>
  </xsl:template>
  <xsl:template match="qa">
    <table border="0" cellpadding="0" cellspacing="0" width="100%">
      <tr>
        <td bgcolor="#999999">
          <table border="0" cellpadding="3px" cellspacing="0" width="100%">
            <tr>
              <td>
                <a>
                  <xsl:attribute name="name">
                    <xsl:value-of select="position() div 2"/>
                  </xsl:attribute>
                  <font color="#FFCC33" size="+1">
                    <xsl:value-of select="question"/>
                  </font>
                </a>
              </td>
            </tr>
          </table>
        </td>
      </tr>
      <tr>
        <td>
          <xsl:apply-templates/>
        </td>
      </tr>
    </table>
    <p>
      <font size="-1">
        <a href="#index">(back to index)</a>
      </font>
    </p>
  </xsl:template>
  <xsl:template match="question"/>
  <xsl:template match="answer">
    <xsl:apply-templates/>
  </xsl:template>
  <xsl:template match="turntabletable">
    <table border="2" cellpadding="2px" width="100%">
      <!--
  
<tr bgcolor="#FFFF99">
<td width="100%" colspan="2">
<font face="Arial,Helvetica" color="#000000" size="+2">
<b>Users' Turntables
</b>
</font>
</td>
</tr>
  -->
      <xsl:apply-templates/>
    </table>
  </xsl:template>
  <xsl:template match="tt">
    <tr bgcolor="#FFFF99">
      <td align="left" colspan="2" width="100%">
        <font color="#000000" face="Arial,Helvetica" size="+1">
          <b>
            <xsl:value-of select="@title"/>
          </b>
        </font>
      </td>
    </tr>
    <tr>
      <td valign="top" width="50%">
        <xsl:for-each select="author">
          <b>Creator:</b>
          <xsl:apply-templates/>
          <br/>
        </xsl:for-each>
        <xsl:for-each select="text">
          <xsl:apply-templates/>
        </xsl:for-each>
      </td>
      <td width="50%">
        <table width="100%">
          <tr>
            <xsl:for-each select="ttimages">
              <xsl:apply-templates/>
            </xsl:for-each>
          </tr>
        </table>
      </td>
    </tr>
  </xsl:template>
  <xsl:template match="scratchtable">
    <table border="2" width="100%">
      <tr bgcolor="#FFFF99">
        <td>
          <font color="#000000" face="Arial,Helvetica" size="+1">
            <b>Scratch</b>
          </font>
        </td>
        <td>
          <font color="#000000" face="Arial,Helvetica" size="+1">
            <b>Author</b>
          </font>
        </td>
        <td>
          <font color="#000000" face="Arial,Helvetica" size="+1">
            <b>Description</b>
          </font>
        </td>
      </tr>
      <xsl:apply-templates/>
    </table>
  </xsl:template>
  <xsl:template match="scratch">
    <tr>
      <xsl:apply-templates/>
    </tr>
  </xsl:template>
  <xsl:template match="mp3file|author|comment">
    <td valign="top">
      <xsl:apply-templates/>
    </td>
  </xsl:template>
  <xsl:template match="tinycomment|filesize">
    <br/>
    <font size="-1">
      <xsl:apply-templates/>
    </font>
  </xsl:template>
  <xsl:template match="screen">
    <span style="font-family: monospace; color: #DDFFDD">
      <xsl:apply-templates/>
    </span>
  </xsl:template>
  <xsl:template match="screenshot">
    <xsl:if test="name(.)='screenshot'">CLOSE_DA_P</xsl:if>
    <center>
      <table border="0" width="100%">
        <tr>
          <td>
            <center>
              <a>
                <xsl:attribute name="href">pix/<xsl:value-of select="@filename"/>
                </xsl:attribute>
                <img border="0">
                  <xsl:attribute name="alt">
                    <xsl:value-of select="name(.)"/>
                  </xsl:attribute>
                  <xsl:attribute name="src">pix/pre_<xsl:value-of select="@filename"/>
                  </xsl:attribute>
                </img>
              </a>
            </center>
          </td>
        </tr>
        <tr>
          <td>
            <center>
              <xsl:apply-templates/>
            </center>
          </td>
        </tr>
      </table>
    </center>
    <xsl:if test="name(.)='screenshot'">OPEN_DA_P</xsl:if>
  </xsl:template>
  <xsl:template match="image">
    <xsl:if test="position()&gt;3">
      <xsl:if test="(position() div 2) mod 3=1">NEW_TT_ROW</xsl:if>
    </xsl:if>
    <td align="center" valign="top" width="33%">
      <table border="0">
        <tr>
          <td>
            <center>
              <a>
                <xsl:attribute name="href">pix/<xsl:value-of select="@filename"/>
                </xsl:attribute>
                <img border="0">
                  <xsl:attribute name="alt">
                    <xsl:value-of select="name(.)"/>
                  </xsl:attribute>
                  <xsl:attribute name="src">pix/pre_<xsl:value-of select="@filename"/>
                  </xsl:attribute>
                </img>
              </a>
            </center>
          </td>
        </tr>
        <tr>
          <td>
            <center>
              <xsl:apply-templates/>
            </center>
          </td>
        </tr>
      </table>
    </td>
  </xsl:template>
  <xsl:template match="ilink">
    <table bgcolor="#999999" border="0" cellspacing="5px" width="100%">
      <tr>
        <xsl:if test="banner">
          <td halign="center" valign="top" width="90px">
            <a>
              <xsl:attribute name="href">
                <xsl:if test="link">
                  <xsl:value-of select="link/@ref"/>
                </xsl:if>
                <xsl:if test="dblink">
                  <xsl:variable name="search" select="dblink/@id"/>
                  <xsl:value-of select="//dlink[@id=$search]/@url"/>
                </xsl:if>
              </xsl:attribute>
              <img border="0">
                <xsl:attribute name="alt">
                  <xsl:value-of select="logo"/>
                </xsl:attribute>
                <xsl:attribute name="src">pix/banner/<xsl:value-of select="banner/@filename"/>
                </xsl:attribute>
              </img>
            </a>
          </td>
        </xsl:if>
        <td valign="top">
          <xsl:for-each select="para">
            <xsl:apply-templates/>
          </xsl:for-each>
        </td>
      </tr>
    </table>
    <xsl:if test="following-sibling::ilink">
      <hr/>
    </xsl:if>
  </xsl:template>
</xsl:stylesheet>
