// EmbedInSource v2018-04-03 (C) 2018 microentropie.com
// Source: 'cFan.html', converted: 2018-04-08 16:22:01

static const char html[3359] PROGMEM =
"<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\r\n"
"<html lang=\"en\">\r\n"
"<head>\r\n"
"   <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\r\n"
"   <link rel=\"stylesheet\" type=\"text/css\" href=\"/style.css\">\r\n"
"   <script type=\"text/javascript\" src=\"/cFan.js\"></script>\r\n"
"   <meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\">\r\n"
"   <title>cFan</title>\r\n"
"</head>\r\n"
"<body onload=\"OnLoadcFanDoc()\" bgcolor=\"#FFFFAA\">\r\n"
"   <h3 id=\"mDesc\">cFan</h3>\r\n"
"   <div><small>id: <span id=\"mId\"></span></small></div>\r\n"
"   <div>Time: <span id=\"dateTime\">00:00:00</span></div>\r\n"
"   <div>Temperature: <span id=\"tmprt\"></span></div>\r\n"
"   <form method=\"POST\" action=\"do\">\r\n"
"      <table id=\"Fan_tbl\" style=\"display: none;\">\r\n"
"         <colgroup>\r\n"
"            <col style=\"width:85px\">\r\n"
"            <col style=\"width:230px\">\r\n"
"         </colgroup>\r\n"
"         <tbody>\r\n"
"            <tr><th colspan=\"2\" align=\"left\">Fan:</th></tr>\r\n"
"            <tr><td id=\"Fan_stl\" style=\"color:white; background-color:blue;\"><b id=\"Fan_sts\">Off</b></td><td>timer: <span id=\"Fan_tmr\">0'00\"</span></td></tr>\r\n"
"            <tr><td colspan=\"2\" align=\"left\"><small>(default at startup: <span id=\"Fan_cfg\"></span>)</small></td></tr>\r\n"
"            <tr>\r\n"
"               <td colspan=\"2\">\r\n"
"                  <button type=\"button\" onclick=\"UpdateTime('Fan',120)\">+2h</button>&nbsp;&nbsp;&nbsp;\r\n"
"                  <button type=\"button\" onclick=\"UpdateTime('Fan',30)\">+30m</button>&nbsp;&nbsp;&nbsp;\r\n"
"                  <button type=\"button\" onclick=\"UpdateTime('Fan',10)\">+10m</button>&nbsp;&nbsp;&nbsp;\r\n"
"                  <button type=\"button\" onclick=\"UpdateTime('Fan','OFF')\">OFF</button>\r\n"
"               </td>\r\n"
"            </tr>\r\n"
"         </tbody>\r\n"
"      </table><br>\r\n"
"      <table id=\"Light_tbl\" style=\"display: none;\">\r\n"
"         <colgroup>\r\n"
"            <col style=\"width:85px\">\r\n"
"            <col style=\"width:230px\">\r\n"
"         </colgroup>\r\n"
"         <tbody>\r\n"
"            <tr><th colspan=\"2\" align=\"left\">Light:</th></tr>\r\n"
"            <tr><td id=\"Light_stl\" style=\"color:white; background-color:blue;\"><b id=\"Light_sts\">Off</b></td><td>timer: <span id=\"Light_tmr\">0'00\"</span></td></tr>\r\n"
"            <tr><td colspan=\"2\" align=\"left\"><small>(default at startup: <span id=\"Light_cfg\"></span>)</small></td></tr>\r\n"
"            <tr>\r\n"
"               <td colspan=\"2\">\r\n"
"                  <button type=\"button\" onclick=\"UpdateTime('Light',120)\">+2h</button>&nbsp;&nbsp;&nbsp;\r\n"
"                  <button type=\"button\" onclick=\"UpdateTime('Light',30)\">+30m</button>&nbsp;&nbsp;&nbsp;\r\n"
"                  <button type=\"button\" onclick=\"UpdateTime('Light',10)\">+10m</button>&nbsp;&nbsp;&nbsp;\r\n"
"                  <button type=\"button\" onclick=\"UpdateTime('Light','OFF')\">OFF</button>\r\n"
"               </td>\r\n"
"            </tr>\r\n"
"         </tbody>\r\n"
"      </table>\r\n"
"      <p>\r\n"
"         Warning:<br />\r\n"
"         <small>\r\n"
"            By pressing 'Shutdown' will bring this device OFFLINE.<br />\r\n"
"            Will need to switch Power Off and then On again to reboot.\r\n"
"            Loads will be switched Off :-).\r\n"
"         </small><br />\r\n"
"         <input name=\"Global\" value=\"Shutdown\" type=\"submit\">\r\n"
"      </p>\r\n"
"   </form>\r\n"
"   <p><small>&copy; 2017-<span id=\"year\">2018</span> <a href=\"http://www.microentropie.com/\">microentropie.com</a></small></p>\r\n"
"</body>\r\n"
"</html>\r\n";
#define SIZEOF_ 3359
#define STRLEN_ 3358