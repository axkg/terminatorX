<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl='http://www.w3.org/1999/XSL/Transform' version='1.0'>
  <xsl:template match='section'>
    <html>
      <head>
        <meta name='Author' content='Alexander Koenig' />
        <meta http-equiv='Content-Type' content='text/html; charset=iso-8859-1' />
        <link rel='icon' href='favicon.ico' type='image/x-icon' />
        <link rel='shortcut icon' href='favicon.ico' type='image/x-icon' />
        <title>terminatorX: 
        <xsl:value-of select='@name' /></title>
        <script language='Javascript' src='script.js' type='text/javascript'></script>
        <style type='text/css'>
        A:hover {text-decoration: none; color: #ff4444}
        A:active {text-decoration: none; color: #ff4444}
        body { font-family: Verdana, sans-serif; }
        p.plain { text-align: left }
        p.fancy { text-align: justify }
        </style>
      </head>
      <body link='#FFFF99' vlink='#FFCC33' alink='#FF0000' bgcolor='#555555'
      text='#FFFFFF'
      onload="rolloverLoad('main','pix/vinyl-hover.gif','pix/vinyl.png'); rolloverLoad('download','pix/vinyl-hover.gif','pix/vinyl.png'); rolloverLoad('screenshots','pix/vinyl-hover.gif','pix/vinyl.png'); rolloverLoad('faq','pix/vinyl-hover.gif','pix/vinyl.png'); rolloverLoad('docs','pix/vinyl-hover.gif','pix/vinyl.png'); rolloverLoad('turntable','pix/vinyl-hover.gif','pix/vinyl.png'); rolloverLoad('scratches','pix/vinyl-hover.gif','pix/vinyl.png'); rolloverLoad('aseqjoy','pix/vinyl-hover.gif','pix/vinyl.png');">

        <table width='100%' cellspacing='5px'>
          <tr>
            <td>
              <center>
                <img src='pix/tX_logo.jpg' ALT='terminatorX' />
              </center>
            </td>
          </tr>
        </table>
<!--new-->
        <table width='100%' cellspacing='0' cellpadding='0' border='0'>
          <tr>
            <td bgcolor='#ffffff' align='left'>
              <font face='Arial,Helvetica' color='#ffffff' size='+2'>
              menu</font>
            </td>
            <td width='100%' bgcolor='#ffffff'>
              <table width='100%' cellspacing='0' cellpadding='4px' border='0'>
                <tr>
                  <td width='100%' bgcolor='#ffffff' align='right'>
                    <font face='Arial,Helvetica' color='#000000' size='+2'>
                      <b>
                        <xsl:value-of select='@name' />
                      </b>
                    </font>
                  </td>
                </tr>
              </table>
            </td>
          </tr>
          <tr>
<!--menu-->
            <td valign='top'>
              <table cellspacing='0' cellpadding='2px' border='0'>
                <tr>
                  <xsl:if test="@name='main'">
                    <td bgcolor='#777777'>
                      <img vspace='0' alt='-' src='pix/vinyl-highlight.png'
                      border='0' />
                    </td>
                    <td bgcolor='#777777'>main</td>
                    <td bgcolor='#777777'>
                      <img vspace='0' alt='-' src='pix/vinyl-spacer.png'
                      border='0' />
                    </td>
                  </xsl:if>
                  <xsl:if test="@name!='main'">
                    <td>
                      <img vspace='0' alt='-' src='pix/vinyl.png' name='main'
                      border='0' />
                    </td>
                    <td>
                      <a onMouseOver="rolloverOn('main');"
                      onMouseOut="rolloverOff('main');" href='./'>main</a>
                    </td>
                    <td />
                  </xsl:if>
                </tr>
                <tr>
                  <xsl:if test="@name='download'">
                    <td bgcolor='#777777'>
                      <img vspace='0' alt='-' src='pix/vinyl-highlight.png'
                      border='0' />
                    </td>
                    <td bgcolor='#777777'>download</td>
                    <td bgcolor='#777777'>
                      <img vspace='0' alt='-' src='pix/vinyl-spacer.png'
                      border='0' />
                    </td>
                  </xsl:if>
                  <xsl:if test="@name!='download'">
                    <td>
                      <img vspace='0' alt='-' src='pix/vinyl.png'
                      name='download' border='0' />
                    </td>
                    <td>
                      <a onMouseOver="rolloverOn('download');"
                      onMouseOut="rolloverOff('download');"
                      href='download.html'>download</a>
                    </td>
                    <td />
                  </xsl:if>
                </tr>
                <tr>
                  <xsl:if test="@name='screenshots'">
                    <td bgcolor='#777777'>
                      <img vspace='0' alt='-' src='pix/vinyl-highlight.png'
                      border='0' />
                    </td>
                    <td bgcolor='#777777'>screenshots</td>
                    <td bgcolor='#777777'>
                      <img vspace='0' alt='-' src='pix/vinyl-spacer.png'
                      border='0' />
                    </td>
                  </xsl:if>
                  <xsl:if test="@name!='screenshots'">
                    <td>
                      <img vspace='0' alt='-' src='pix/vinyl.png'
                      name='screenshots' border='0' />
                    </td>
                    <td>
                      <a onMouseOver="rolloverOn('screenshots');"
                      onMouseOut="rolloverOff('screenshots');"
                      href='screenshots.html'>screenshots</a>
                    </td>
                    <td />
                  </xsl:if>
                </tr>
                <tr>
                  <xsl:if test="@name='faq'">
                    <td bgcolor='#777777'>
                      <img vspace='0' alt='-' src='pix/vinyl-highlight.png'
                      border='0' />
                    </td>
                    <td bgcolor='#777777'>faq</td>
                    <td bgcolor='#777777'>
                      <img vspace='0' alt='-' src='pix/vinyl-spacer.png'
                      border='0' />
                    </td>
                  </xsl:if>
                  <xsl:if test="@name!='faq'">
                    <td>
                      <img vspace='0' alt='-' src='pix/vinyl.png' name='faq'
                      border='0' />
                    </td>
                    <td>
                      <a onMouseOver="rolloverOn('faq');"
                      onMouseOut="rolloverOff('faq');" href='faq.html'>faq</a>
                    </td>
                    <td />
                  </xsl:if>
                </tr>
                <tr>
                  <xsl:if test="@name='docs'">
                    <td bgcolor='#777777'>
                      <img vspace='0' alt='-' src='pix/vinyl-highlight.png'
                      border='0' />
                    </td>
                    <td bgcolor='#777777'>docs</td>
                    <td bgcolor='#777777'>
                      <img vspace='0' alt='-' src='pix/vinyl-spacer.png'
                      border='0' />
                    </td>
                  </xsl:if>
                  <xsl:if test="@name!='docs'">
                    <td>
                      <img vspace='0' alt='-' src='pix/vinyl.png' name='docs'
                      border='0' />
                    </td>
                    <td>
                      <a onMouseOver="rolloverOn('docs');"
                      onMouseOut="rolloverOff('docs');" href='docs.html'>
                      docs</a>
                    </td>
                    <td />
                  </xsl:if>
                </tr>
                <tr>
                  <xsl:if test="@name='scratches'">
                    <td bgcolor='#777777'>
                      <img vspace='0' alt='-' src='pix/vinyl-highlight.png'
                      border='0' />
                    </td>
                    <td bgcolor='#777777'>scratches</td>
                    <td bgcolor='#777777'>
                      <img vspace='0' alt='-' src='pix/vinyl-spacer.png'
                      border='0' />
                    </td>
                  </xsl:if>
                  <xsl:if test="@name!='scratches'">
                    <td>
                      <img vspace='0' alt='-' src='pix/vinyl.png'
                      name='scratches' border='0' />
                    </td>
                    <td>
                      <a onMouseOver="rolloverOn('scratches');"
                      onMouseOut="rolloverOff('scratches');"
                      href='scratches.html'>scratches</a>
                    </td>
                    <td />
                  </xsl:if>
                </tr>
                <tr>
                  <xsl:if test="@name='turntable gallery'">
                    <td bgcolor='#777777'>
                      <img vspace='0' alt='-' src='pix/vinyl-highlight.png'
                      border='0' />
                    </td>
                    <td bgcolor='#777777'>turntables</td>
                    <td bgcolor='#777777'>
                      <img vspace='0' alt='-' src='pix/vinyl-spacer.png'
                      border='0' />
                    </td>
                  </xsl:if>
                  <xsl:if test="@name!='turntable gallery'">
                    <td>
                      <img vspace='0' alt='-' src='pix/vinyl.png'
                      name='turntable' border='0' />
                    </td>
                    <td>
                      <a onMouseOver="rolloverOn('turntable');"
                      onMouseOut="rolloverOff('turntable');"
                      href='turntable.html'>turntables</a>
                    </td>
                    <td />
                  </xsl:if>
                </tr>
                <tr>
                  <xsl:if test="@name='aseqjoy'">
                    <td bgcolor='#777777'>
                      <img vspace='0' alt='-' src='pix/vinyl-highlight.png'
                      border='0' />
                    </td>
                    <td bgcolor='#777777'>aseqjoy</td>
                    <td bgcolor='#777777'>
                      <img vspace='0' alt='-' src='pix/vinyl-spacer.png'
                      border='0' />
                    </td>
                  </xsl:if>
                  <xsl:if test="@name!='aseqjoy'">
                    <td>
                      <img vspace='0' alt='-' src='pix/vinyl.png'
                      name='aseqjoy' border='0' />
                    </td>
                    <td>
                      <a onMouseOver="rolloverOn('aseqjoy');"
                      onMouseOut="rolloverOff('aseqjoy');" href='aseqjoy.html'>
                      aseqjoy</a>
                    </td>
                    <td />
                  </xsl:if>
                </tr>
              </table>
            </td>
<!--contents-->
            <td valign='top'>
              <table width='100%' cellspacing='0' cellpadding='10px'
              border='0'>
                <tr>
                  <td width='100%' bgcolor='#777777'>
                    <xsl:apply-templates />
                  </td>
                </tr>
              </table>
            </td>
          </tr>
        </table>
<!-- bottom -->
<HR width='100%' />
        <table width='100%'>
          <tr>
            <td width='100%' align='left' valign='top'>
              <font size='-1'>Copyright (C) 1999-2003 by 
              <A HREF='mailto:alex@lisas.de'>Alexander
              KMAKE_THIS_MYOUMLnig</A></font>
            </td>
            <td>
              <a href='http://validator.w3.org/check/referer'>
                <img width='88' alt='Valid HTML 4.01!'
                src='pix/valid-html401.png' height='31'
                border='0' />
              </a>
            </td>
          </tr>
        </table>
      </body>
    </html>
  </xsl:template>
  <xsl:template match='heading'>
    <font size='+2'>
      <xsl:apply-templates />
    </font>
    <br />
  </xsl:template>
  <xsl:template match='subtitle'>
    <font size='-2'>
      <xsl:apply-templates />
    </font>
  </xsl:template>
  <xsl:template match='para'>
    <xsl:if test="@class='fancy'">
      <p class='fancy'>
        <xsl:apply-templates />
      </p>
    </xsl:if>
    <xsl:if test="@class='plain'">
      <p class='plain'>
        <xsl:apply-templates />
      </p>
    </xsl:if>
  </xsl:template>
  <xsl:template match='italic'>
    <i>
      <xsl:apply-templates />
    </i>
  </xsl:template>
  <xsl:template match='red'>
    <font color='#FF4444'>
      <xsl:apply-templates />
    </font>
  </xsl:template>
  <xsl:template match='bold'>
    <b>
      <xsl:apply-templates />
    </b>
  </xsl:template>
  <xsl:template match='newsheader' />
  <xsl:template match='newsitem'>
    <table cellspacing='0' cellpadding='0' border='0'>
      <!-- header -->
<tr>
        <td bgcolor='#999999'>
          <xsl:for-each select='newsheader'>
            <table cellspacing='0' cellpadding='2px' border='0'>
              <tr>
                <td>
                  <img alt='new:' src='pix/new.png' />
                </td>
                <td width='100%' align='left' valign='middle'>
                  <font size='+1'>
                    <xsl:apply-templates />
                  </font> <font color='#DDDDDD' size='+1'> 
                  [<xsl:value-of select='@date' />]</font>
                </td>
              </tr>
            </table>
          </xsl:for-each>
        </td>
      </tr>
      <!-- contents -->
<tr>
        <td>
          <xsl:apply-templates />
        </td>
      </tr>
    </table>
  </xsl:template>
  <xsl:template match='link'>
    <A>
      <xsl:attribute name='HREF'>
        <xsl:value-of select='@ref' />
      </xsl:attribute>
      <xsl:apply-templates />
    </A>
  </xsl:template>
  <xsl:template match='newline'>
    <br />
  </xsl:template>
  <xsl:template match='itemizedlist'>CLOSE_DA_P 
  <ul>
    <xsl:apply-templates />
  </ul>OPEN_DA_P</xsl:template>
  <xsl:template match='listitem'>
    <li>
      <xsl:apply-templates />
    </li>
  </xsl:template>
  <xsl:template match='person'>
    <a>
      <xsl:attribute name='href'>mailto: 
      <xsl:if test="email='alex@lisas.de'">alex@lisas.de</xsl:if>
      <xsl:if test="email!='alex@lisas.de'">NOSPAM_ 
      <xsl:value-of select="substring-before(email, '@')" />_AT_ 
      <xsl:value-of select="substring-after(email, '@')" />_NOSPAM</xsl:if></xsl:attribute>
      <xsl:value-of select='name' />
    </a>
  </xsl:template>
  <xsl:template match='index'>
    <p>
      <font size='+1'>
        <xsl:value-of select='title' />
      </font>
    </p>
    <ul>
      <xsl:for-each select='//subsection'>
        <li>
          <a>
            <xsl:attribute name='href'># 
            <xsl:value-of select='title' /></xsl:attribute>
            <xsl:value-of select='title' />
          </a>
        </li>
      </xsl:for-each>
    </ul>
    <p></p>
  </xsl:template>
  <xsl:template match='subsection'>
    <p>
      <font size='+1'>
        <a>
          <xsl:attribute name='name'>
            <xsl:value-of select='title' />
          </xsl:attribute>
          <xsl:value-of select='title' />
        </a>
      </font>
    </p>
    <xsl:apply-templates />
  </xsl:template>
  <xsl:template match='title'></xsl:template>
  <xsl:template match='tarfiles'>
    <ul>
      <xsl:for-each select='tar'>
        <xsl:if test='@version!=//current/@version'>
          <li>Version 
          <xsl:value-of select='@version' />: 
          <a>
          <xsl:attribute name='href'>terminatorX-<xsl:value-of select='@version' />.tar.gz</xsl:attribute>terminatorX-<xsl:value-of select='@version' />.tar.gz</a>
          <xsl:if test="@havebz2='yes'"> (<a>
          <xsl:attribute name='href'>terminatorX-<xsl:value-of select='@version' />.tar.bz2</xsl:attribute>bz2</a>)</xsl:if>
	 </li>
        </xsl:if>
      </xsl:for-each>
    </ul>
  </xsl:template>
  <xsl:template name='rpm' match='rpm'>
    <li>
      <xsl:if test="@type='src'">Source RPM: </xsl:if>
      <xsl:if test="@type!='src'">
      <xsl:value-of select='@type' />-binary RPM: </xsl:if>
      <a>
      <xsl:attribute name='href'>terminatorX-<xsl:value-of select='@version' />-<xsl:value-of select='@rpmsubversion' />.<xsl:value-of select='@type' />.<xsl:if test='@ext'><xsl:value-of select='@ext' />.</xsl:if>rpm</xsl:attribute>
        terminatorX-<xsl:value-of select='@version' />-<xsl:value-of select='@rpmsubversion' />.<xsl:value-of select='@type' />.<xsl:if test='@ext'><xsl:value-of select='@ext' />.</xsl:if>rpm</a>
      <xsl:if test='@distribution'> built for: <i>
        <xsl:value-of select='@distribution' />
      </i></xsl:if>
      <xsl:if test='@note'> <b>note:</b>  
      <xsl:value-of select='@note' /></xsl:if>
    </li>
  </xsl:template>
  <xsl:template match='rpmfiles'>
    <ul>
      <xsl:for-each select='rpm[@version!=//current/@version]'>
        <xsl:call-template name='rpm' />
      </xsl:for-each>
    </ul>
  </xsl:template>
  <xsl:template match='currentversion'>
    <p>The current terminatorX release is Version 
    <xsl:value-of select='//current/@version' />. 
    <br />Download as:</p>
    <ul>
      <li>tar file: 
      <a>
      <xsl:attribute name='href'>terminatorX-<xsl:value-of select='//current/@version' />.tar.gz</xsl:attribute>terminatorX-<xsl:value-of select='//current/@version' />.tar.gz</a>
      <xsl:if test="//current/@havebz2='yes'"> (<a><xsl:attribute name='href'>terminatorX-<xsl:value-of select='//current/@version' />.tar.bz2</xsl:attribute>bz2</a>)</xsl:if>
      </li>
      <xsl:for-each select='//rpm[@version=//current/@version]'>
        <xsl:call-template name='rpm' />
      </xsl:for-each>
    </ul>
    <xsl:if test='sum(//rpm[@version=//current/@version])=0'>Sorry, no RPMs
    available for the current release, yet. Stay tuned.</xsl:if>
  </xsl:template>
  <xsl:template match='filelist'>
    <ul>
      <xsl:for-each select='file'>
        <li>
          <a>
            <xsl:attribute name='href'>
              <xsl:apply-templates />
            </xsl:attribute>
            <xsl:apply-templates />
          </a>
        </li>
      </xsl:for-each>
    </ul>
  </xsl:template>
  <xsl:template match='file'>
    <a>
      <xsl:attribute name='href'>
        <xsl:apply-templates />
      </xsl:attribute>
      <xsl:apply-templates />
    </a>
  </xsl:template>
  <xsl:template match='faq'>
    <a name='index'>
      <font size='+1'>Question-Index:</font>
    </a>
    <ul>
      <xsl:for-each select='qa'>
        <li>
          <a>
            <xsl:attribute name='href'># 
            <xsl:value-of select='position()' /></xsl:attribute>
            <xsl:value-of select='question' />
          </a>
        </li>
      </xsl:for-each>
    </ul>
    <p>
      <font size='+1'>Answers:</font>
    </p>
    <xsl:apply-templates />
  </xsl:template>
  <xsl:template match='qa'>
    <table width='100%' cellspacing='0' cellpadding='0' border='0'>
      <tr>
        <td bgcolor='#999999'>
          <table width='100%' cellspacing='0' cellpadding='3px' border='0'>
            <tr>
              <td>
                <a>
                  <xsl:attribute name='name'>
                    <xsl:value-of select='position() div 2' />
                  </xsl:attribute>
                  <font color='#FFCC33' size='+1'>
                    <xsl:value-of select='question' />
                  </font>
                </a>
              </td>
            </tr>
          </table>
        </td>
      </tr>
      <tr>
        <td>
          <xsl:apply-templates />
        </td>
      </tr>
    </table>
    <p>
      <font size='-1'>
        <a href='#index'>(back to index)</a>
      </font>
    </p>
  </xsl:template>
  <xsl:template match='question'></xsl:template>
  <xsl:template match='answer'>
    <xsl:apply-templates />
  </xsl:template>
  <xsl:template match='turntabletable'>
    <table width='100%' cellpadding='2px' border='2'>
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
<xsl:apply-templates />
    </table>
  </xsl:template>
  <xsl:template match='tt'>
    <tr bgcolor='#FFFF99'>
      <td width='100%' colspan='2' align='left'>
        <font face='Arial,Helvetica' color='#000000' size='+1'>
          <b>
            <xsl:value-of select='@title' />
          </b>
        </font>
      </td>
    </tr>
    <tr>
      <td width='50%' valign='top'>
        <xsl:for-each select='author'>
          <b>Creator:</b>
          <xsl:apply-templates />
          <br />
        </xsl:for-each>
        <xsl:for-each select='text'>
          <xsl:apply-templates />
        </xsl:for-each>
      </td>
      <td width='50%'>
        <table width='100%'>
          <tr>
            <xsl:for-each select='ttimages'>
              <xsl:apply-templates />
            </xsl:for-each>
          </tr>
        </table>
      </td>
    </tr>
  </xsl:template>
  <xsl:template match='scratchtable'>
    <table width='100%' border='2'>
      <tr bgcolor='#FFFF99'>
        <td>
          <font face='Arial,Helvetica' color='#000000' size='+1'>
            <b>Scratch</b>
          </font>
        </td>
        <td>
          <font face='Arial,Helvetica' color='#000000' size='+1'>
            <b>Author</b>
          </font>
        </td>
        <td>
          <font face='Arial,Helvetica' color='#000000' size='+1'>
            <b>Description</b>
          </font>
        </td>
      </tr>
      <xsl:apply-templates />
    </table>
  </xsl:template>
  <xsl:template match='scratch'>
    <tr>
      <xsl:apply-templates />
    </tr>
  </xsl:template>
  <xsl:template match='mp3file|author|comment'>
    <td valign='top'>
      <xsl:apply-templates />
    </td>
  </xsl:template>
  <xsl:template match='tinycomment|filesize'>
    <br />
    <font size='-1'>
      <xsl:apply-templates />
    </font>
  </xsl:template>
  <xsl:template match='screen'>
    <span style='font-family: monospace; color: #DDFFDD'>
      <xsl:apply-templates />
    </span>
  </xsl:template>
  <xsl:template match='screenshot'>
    <xsl:if test="name(.)='screenshot'">CLOSE_DA_P</xsl:if>
    <center>
      <table width='100%' border='0'>
        <tr>
          <td>
            <center>
              <a>
                <xsl:attribute name='href'>pix/ 
                <xsl:value-of select='@filename' /></xsl:attribute>
                <img border='0'>
                  <xsl:attribute name='alt'>
                    <xsl:value-of select='name(.)' />
                  </xsl:attribute>
                  <xsl:attribute name='src'>pix/pre_ 
                  <xsl:value-of select='@filename' /></xsl:attribute>
                </img>
              </a>
            </center>
          </td>
        </tr>
        <tr>
          <td>
            <center>
              <xsl:apply-templates />
            </center>
          </td>
        </tr>
      </table>
    </center>
    <xsl:if test="name(.)='screenshot'">OPEN_DA_P</xsl:if>
  </xsl:template>
  <xsl:template match='image'>
    <xsl:if test='position()&gt;3'>
      <xsl:if test='(position() div 2) mod 3=1'>NEW_TT_ROW</xsl:if>
    </xsl:if>
    <td width='33%' align='center' valign='top'>
      <table border='0'>
        <tr>
          <td>
            <center>
              <a>
                <xsl:attribute name='href'>pix/ 
                <xsl:value-of select='@filename' /></xsl:attribute>
                <img border='0'>
                  <xsl:attribute name='alt'>
                    <xsl:value-of select='name(.)' />
                  </xsl:attribute>
                  <xsl:attribute name='src'>pix/pre_ 
                  <xsl:value-of select='@filename' /></xsl:attribute>
                </img>
              </a>
            </center>
          </td>
        </tr>
        <tr>
          <td>
            <center>
              <xsl:apply-templates />
            </center>
          </td>
        </tr>
      </table>
    </td>
  </xsl:template>
</xsl:stylesheet>
