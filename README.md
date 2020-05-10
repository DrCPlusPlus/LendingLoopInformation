# LendingLoopInformation
A program for manipulating and interpreting the payment schedule from Lending Loop

Requirements:<br/>
&nbsp;&nbsp;&nbsp;Curl<br/>
&nbsp;&nbsp;&nbsp;Gtk+3<br/>

Build Requirements:<br/>
&nbsp;&nbsp;&nbsp;Build essentials<br/>
&nbsp;&nbsp;&nbsp;PkgConfig<br/>
&nbsp;&nbsp;&nbsp;CMake<br/>
&nbsp;&nbsp;&nbsp;libcurl-dev - use whatever flavour of SSL you'd like (libcurl4-openssl-dev for example)<br/>
&nbsp;&nbsp;&nbsp;libgtk-3-dev<br/>
  

Instructions:<br/>
&nbsp;&nbsp;&nbsp;mkdir build<br/>
&nbsp;&nbsp;&nbsp;cd build<br/>
&nbsp;&nbsp;&nbsp;cmake ..<br/>
&nbsp;&nbsp;&nbsp;make<br/>
  
Building the deb package:<br/>
&nbsp;&nbsp;&nbsp;dpkg-deb --build LendingLoopInformationDeb<br/>


Install Deb package:<br/>
dpkg -i LendingLoopInformationDeb.deb
  
