<h1><b><span style='font-size:40.0pt;
line-height:300%'>projanitor</span></b></h1>
  

<p class=MsoNormal><o:p>&nbsp;</o:p></p> 

<p class=MsoNormal><span class=SpellE><b>projanitor</b></span><b> </b>is a
lightweight Python tool that audits the integrity of software development
projects, with a focus on embedded systems and build environments like <span
class=SpellE>CMake</span>. Developed for an embedded system project, it
identifies the project root, catalogs relevant files, and generates a detailed
report on duplicate, orphaned, and missing source files. Keep your project
clean and organized with minimal effort.</p>

<h1>Features</h1>

<ul style='margin-top:0in' type=disc>
 <li class=MsoNormal style='mso-list:l1 level1 lfo1;tab-stops:list .5in'><b>Project
     Root Detection</b>: Automatically locates the project root using marker
     files (e.g., <span class=CodeChar><span style='font-size:10.0pt;
     mso-bidi-font-size:12.0pt;line-height:115%'>CMakeLists.txt, LICENSE</span></span>).</li>
 <li class=MsoNormal style='mso-list:l1 level1 lfo1;tab-stops:list .5in'><b>File
     Auditing</b>: Scans for files of interest (e.g., <span class=CodeChar><span
     style='font-size:10.0pt;mso-bidi-font-size:12.0pt;line-height:115%'>.c,
     .h, .<span class=SpellE>py</span><span class=GramE>, .<span class=SpellE>cmake</span></span></span></span>)
     and reports duplicates, orphans (unreferenced files), and missing files
     (referenced but not found).</li>
 <li class=MsoNormal style='mso-list:l1 level1 lfo1;tab-stops:list .5in'><b>Configurable</b>:
     Customize file extensions, excluded directories, and marker files via
     command-line arguments.</li>
 <li class=MsoNormal style='mso-list:l1 level1 lfo1;tab-stops:list .5in'><b>Verbose
     Logging</b>: Optional detailed warnings for debugging issues like
     unreadable files or <span class=SpellE>symlinks</span>.</li>
 <li class=MsoNormal style='mso-list:l1 level1 lfo1;tab-stops:list .5in'><b>Efficient</b>:
     Single-pass directory traversal and line-by-line file reading for
     performance with large projects.</li>
</ul>

<h1>Requirements</h1>

<ul style='margin-top:0in' type=disc>
 <li class=MsoNormal style='mso-list:l0 level1 lfo2;tab-stops:list .5in'>Python
     3.6 or higher</li>
 <li class=MsoNormal style='mso-list:l0 level1 lfo2;tab-stops:list .5in'>No
     external dependencies (uses standard library modules: <span class=SpellE><span
     class=CodeChar><span style='font-size:10.0pt;mso-bidi-font-size:12.0pt;
     line-height:115%'>os</span></span></span><span class=CodeChar><span
     style='font-size:10.0pt;mso-bidi-font-size:12.0pt;line-height:115%'>, <span
     class=SpellE>pathlib</span>, <span class=SpellE>argparse</span>, re,
     collections, typing</span></span>)</li>
</ul>

<h1>Installation</h1>

<ul style='margin-top:0in' type=disc>
 <li class=MsoNormal style='mso-list:l4 level1 lfo3;tab-stops:list .5in'><b>Clone
     the Repository</b>:</li>
</ul>

<p class=Code style='margin-left:.5in'>git clone https://github.com/dyfcheng/projanitor.git</p>

<p class=Code style='margin-left:.5in'>cd <span class=SpellE>projanitor</span></p>

<ul style='margin-top:0in' type=disc>
 <li class=MsoNormal style='mso-list:l4 level1 lfo3;tab-stops:list .5in'><b>Copy
     to Project</b> (optional): <br>
     For use in <span class=GramE>the <span class=SpellE>ProjectA</span></span>
     project:</li>
</ul>

<p class=Code style='margin-left:.5in'>cp projanitor.py /home/tom/work/<span
class=SpellE>ProjectA</span>/<span class=SpellE>ProjectA_build_scripts</span>/<span
class=SpellE>House_cleaning</span>/</p>

<ul style='margin-top:0in' type=disc>
 <li class=MsoNormal style='mso-list:l4 level1 lfo3;tab-stops:list .5in'><b>Make
     Executable</b> (optional):</li>
</ul>

<p class=Code style='margin-left:.5in'><span class=SpellE>chmod</span> +x
projanitor.py</p>

<h1>Usage</h1>

<p class=MsoNormal>Run <span class=SpellE><span class=GramE><b>projanitor</b></span></span>
from your project directory to audit files and generate a report.</p>

<h1>Quick Start</h1>

<p class=MsoNormal>For <span class=GramE>a<span style='mso-spacerun:yes'> 
</span>project</span> named <span class=SpellE><span class=CodeChar><span
style='font-size:10.0pt;mso-bidi-font-size:12.0pt;line-height:115%'>ProjectA</span></span></span>
owned by <span class=CodeChar><span style='font-size:10.0pt;mso-bidi-font-size:
12.0pt;line-height:115%'>tom</span></span> in <span class=CodeChar><span
style='font-size:10.0pt;mso-bidi-font-size:12.0pt;line-height:115%'>~/work/</span></span>
directory:</p>

<p class=Code style='margin-left:.5in'>cd /home/tom/work/<span class=SpellE>ProjectA</span>/
</p>

<p class=Code style='margin-left:.5in'>python projanitor.py</p>

<p class=MsoNormal>This uses default settings:</p>

<ul style='margin-top:0in' type=disc>
 <li class=MsoNormal style='mso-list:l5 level1 lfo4;tab-stops:list .5in'><b>File
     extensions</b>: <span class=CodeChar><span style='font-size:10.0pt;
     mso-bidi-font-size:12.0pt;line-height:115%'>.c, .h<span class=GramE>, .<span
     class=SpellE>json</span></span>, .<span class=SpellE>py</span><span
     class=GramE>, .<span class=SpellE>cmake</span></span>, .md, .<span
     class=SpellE>sh</span>, CMakeLists.txt</span></span></li>
 <li class=MsoNormal style='mso-list:l5 level1 lfo4;tab-stops:list .5in'><b>Excluded
     directories</b><span class=GramE>: <span class=CodeChar><span
     style='font-size:10.0pt;mso-bidi-font-size:12.0pt;line-height:115%'>.git</span></span></span><span
     class=CodeChar><span style='font-size:10.0pt;mso-bidi-font-size:12.0pt;
     line-height:115%'>, build, <span class=SpellE>build_logs</span>, doc</span></span></li>
 <li class=MsoNormal style='mso-list:l5 level1 lfo4;tab-stops:list .5in'><b>Marker
     files</b>: <span class=CodeChar><span style='font-size:10.0pt;mso-bidi-font-size:
     12.0pt;line-height:115%'>LICENSE, <span class=SpellE>sdkconfig</span>, <span
     class=SpellE><span class=GramE>dependencies.lock</span></span>,
     CMakeLists.txt</span></span></li>
</ul>

<h1>Customizing Options</h1>

<p class=MsoNormal>Tailor the audit with command-line arguments:</p>

<p class=Code>python projanitor.py --extensions=<span class=SpellE><span
class=GramE>c,h</span>,py</span> --exclude-<span class=SpellE>dirs</span><span
class=GramE>=.<span class=SpellE>git</span></span><span class=SpellE>,build</span>
--marker-files=CMakeLists.txt --verbose</p>

<ul style='margin-top:0in' type=disc>
 <li class=MsoNormal style='mso-list:l3 level1 lfo5;tab-stops:list .5in'><span
     class=CodeChar><span style='font-size:10.0pt;mso-bidi-font-size:12.0pt;
     line-height:115%'>--extensions</span></span>: Comma-separated file
     extensions or names (e.g., <span class=GramE><span class=CodeChar><span
     style='font-size:10.0pt;mso-bidi-font-size:12.0pt;line-height:115%'>c,h</span></span></span><span
     class=CodeChar><span style='font-size:10.0pt;mso-bidi-font-size:12.0pt;
     line-height:115%'>,<span class=GramE>py,CMakeLists.txt</span></span></span>).</li>
 <li class=MsoNormal style='mso-list:l3 level1 lfo5;tab-stops:list .5in'><span
     class=CodeChar><span style='font-size:10.0pt;mso-bidi-font-size:12.0pt;
     line-height:115%'>--exclude-<span class=SpellE>dirs</span></span></span>:
     Comma-separated directories to exclude (e.g.<span class=GramE>, <span
     class=CodeChar><span style='font-size:10.0pt;mso-bidi-font-size:12.0pt;
     line-height:115%'>.<span class=SpellE>git</span></span></span></span><span
     class=SpellE><span class=CodeChar><span style='font-size:10.0pt;
     mso-bidi-font-size:12.0pt;line-height:115%'>,build</span></span></span>).</li>
 <li class=MsoNormal style='mso-list:l3 level1 lfo5;tab-stops:list .5in'><span
     class=CodeChar><span style='font-size:10.0pt;mso-bidi-font-size:12.0pt;
     line-height:115%'>--marker-files</span></span>: Comma-separated files to
     identify the project root (e.g., <span class=CodeChar><span
     style='font-size:10.0pt;mso-bidi-font-size:12.0pt;line-height:115%'>CMakeLists.txt,
     LICENSE</span></span>).</li>
 <li class=MsoNormal style='mso-list:l3 level1 lfo5;tab-stops:list .5in'><span
     class=CodeChar><span style='font-size:10.0pt;mso-bidi-font-size:12.0pt;
     line-height:115%'>--verbose</span></span>: Enable detailed warnings (e.g.,
     skipped <span class=SpellE>symlinks</span>, unreadable files).</li>
</ul>

<h1>Example Output</h1>

<p class=Code>Analyzing project files...</p>

<p class=Code>Project root found at: /home//work/<span class=SpellE>ProjectA</span></p>

<p class=Code><o:p>&nbsp;</o:p></p>

<p class=Code>=== Summary ===</p>

<p class=Code>Project name: <span class=SpellE>ProjectA</span></p>

<p class=Code>Project root folder: /home/tom/work/<span class=SpellE>ProjectA</span></p>

<p class=Code>Key subfolders:</p>

<p class=Code><span style='mso-spacerun:yes'>  </span>- <span class=SpellE>src</span>:
/home/tom/work/<span class=SpellE>ProjectA</span>/<span class=SpellE>src</span></p>

<p class=Code><span style='mso-spacerun:yes'>  </span>- include: /home/tom/work/<span
class=SpellE>ProjectA</span>/include</p>

<p class=Code>Excluded directories:</p>

<p class=Code><span style='mso-spacerun:yes'>  </span>- /home/tom/work/<span
class=SpellE>ProjectA</span><span class=GramE>/.git</span></p>

<p class=Code><span style='mso-spacerun:yes'>  </span>- /home/tom/work/<span
class=SpellE>ProjectA</span>/build</p>

<p class=Code><o:p>&nbsp;</o:p></p>

<p class=Code>=== Statistics ===</p>

<p class=Code>Total # of files of interest: 10</p>

<p class=Code># of .c: 4</p>

<p class=Code># of .h: 3</p>

<p class=Code># <span class=GramE>of .<span class=SpellE>cmake</span></span>: 1</p>

<p class=Code># of .<span class=SpellE>sh</span>: 0</p>

<p class=Code># <span class=GramE>of .<span class=SpellE>json</span></span>: 0</p>

<p class=Code># of .<span class=SpellE>py</span>: 2</p>

<p class=Code># of .md: 0</p>

<p class=Code># of CMakeLists.txt: 1</p>

<p class=Code><o:p>&nbsp;</o:p></p>

<p class=Code>=== Warnings ===</p>

<p class=Code>.c files with identical names:</p>

<p class=Code><span style='mso-spacerun:yes'>  </span><span class=SpellE>main.c</span>:
/home/tom/work/<span class=SpellE>ProjectA</span>/<span class=SpellE>src</span>/<span
class=SpellE>main<span class=GramE>.c</span></span></p>

<p class=Code><span style='mso-spacerun:yes'>         </span>/home/tom/work/<span
class=SpellE>ProjectA</span>/backup/<span class=SpellE>main<span class=GramE>.c</span></span></p>

<p class=Code>.h files with identical names:</p>

<p class=Code><span style='mso-spacerun:yes'>  </span>(None)</p>

<p class=Code>.<span class=SpellE>py</span> files with identical names:</p>

<p class=Code><span style='mso-spacerun:yes'>  </span>(None)</p>

<p class=Code>.<span class=SpellE>sh</span> files with identical names:</p>

<p class=Code><span style='mso-spacerun:yes'>  </span>(None)</p>

<p class=Code><o:p>&nbsp;</o:p></p>

<p class=Code>=== Errors ===</p>

<p class=Code># of orphan files: 1</p>

<p class=Code>Details of Orphan Files:</p>

<p class=Code>- /home/tom/work/<span class=SpellE>ProjectA</span>/<span
class=SpellE>src</span>/<span class=SpellE>utils<span class=GramE>.c</span></span></p>

<p class=Code><o:p>&nbsp;</o:p></p>

<p class=Code># of missing files: 1</p>

<p class=Code>Details of Missing Files:</p>

<p class=Code>- <span class=SpellE>config.h</span></p>

<p class=Code><span style='mso-spacerun:yes'>  </span>referenced by:</p>

<p class=Code><span style='mso-spacerun:yes'>    </span>- /home/tom/work/<span
class=SpellE>ProjectA</span>/<span class=SpellE>src</span>/<span class=SpellE>main<span
class=GramE>.c</span></span></p>

<h1>Project Structure</h1>

<p class=MsoNormal><span class=SpellE><b>projanitor</b></span> is designed for
projects like <span class=SpellE><span class=CodeChar><span style='font-size:
10.0pt;mso-bidi-font-size:12.0pt;line-height:115%'>ProjectA</span></span></span>,
which likely involve embedded systems <span class=GramE>with <span
class=CodeChar><span style='font-size:10.0pt;mso-bidi-font-size:12.0pt;
line-height:115%'>.c</span></span></span><span class=CodeChar><span
style='font-size:10.0pt;mso-bidi-font-size:12.0pt;line-height:115%'>, .h<span
class=GramE>, .<span class=SpellE>cmake</span></span>,</span></span> and <span
class=CodeChar><span style='font-size:10.0pt;mso-bidi-font-size:12.0pt;
line-height:115%'>CMakeLists.txt</span></span> files. It identifies the project
root using marker files and scans subdirectories (excluding specified ones) to
analyze file references, such as <span class=CodeChar><span style='font-size:
10.0pt;mso-bidi-font-size:12.0pt;line-height:115%'>#include</span></span> in <span
class=CodeChar><span style='font-size:10.0pt;mso-bidi-font-size:12.0pt;
line-height:115%'>.<span class=GramE>c/.</span>h</span></span>, imports in <span
class=CodeChar><span style='font-size:10.0pt;mso-bidi-font-size:12.0pt;
line-height:115%'>.<span class=SpellE>py</span></span></span>, or file
references <span class=GramE>in <span class=CodeChar><span style='font-size:
10.0pt;mso-bidi-font-size:12.0pt;line-height:115%'>.<span class=SpellE>cmake</span></span></span></span>/
<span class=CodeChar><span style='font-size:10.0pt;mso-bidi-font-size:12.0pt;
line-height:115%'>CMakeLists.txt</span></span>.</p>

<p class=MsoNormal>Example <span class=SpellE><span class=CodeChar><span
style='font-size:10.0pt;mso-bidi-font-size:12.0pt;line-height:115%'>ProjectA</span></span></span>
structure:</p>

<p class=Code><span class=SpellE>ProjectA</span>/</p>

<p class=Code><span lang=ZH-CN style='font-family:"PMingLiU",serif;mso-bidi-font-family:
PMingLiU'>├</span><span style='font-family:"Aptos",sans-serif;mso-bidi-font-family:
Aptos'>──</span> <span class=SpellE>src</span>/</p>

<p class=Code>│<span style='mso-spacerun:yes'>   </span><span lang=ZH-CN
style='font-family:"PMingLiU",serif;mso-bidi-font-family:PMingLiU'>├</span><span
style='font-family:"Aptos",sans-serif;mso-bidi-font-family:Aptos'>──</span> <span
class=SpellE>main.c</span></p>

<p class=Code>│<span style='mso-spacerun:yes'>   </span>└── <span class=SpellE>utils.c</span></p>

<p class=Code><span lang=ZH-CN style='font-family:"PMingLiU",serif;mso-bidi-font-family:
PMingLiU'>├</span><span style='font-family:"Aptos",sans-serif;mso-bidi-font-family:
Aptos'>──</span> include/</p>

<p class=Code>│<span style='mso-spacerun:yes'>   </span>└── <span class=SpellE>config.h</span></p>

<p class=Code><span lang=ZH-CN style='font-family:"PMingLiU",serif;mso-bidi-font-family:
PMingLiU'>├</span><span style='font-family:"Aptos",sans-serif;mso-bidi-font-family:
Aptos'>──</span> CMakeLists.txt</p>

<p class=Code><span lang=ZH-CN style='font-family:"PMingLiU",serif;mso-bidi-font-family:
PMingLiU'>├</span><span style='font-family:"Aptos",sans-serif;mso-bidi-font-family:
Aptos'>──</span> build/<span style='mso-spacerun:yes'>  </span>(excluded)</p>

<p class=Code>└─<span class=GramE>─ .git</span>/<span class=GramE><span
style='mso-spacerun:yes'>   </span>(</span>excluded)</p>

<h1>Contributing<br style='mso-special-character:line-break'>
<![if !supportLineBreakNewLine]><br style='mso-special-character:line-break'>
<![endif]></h1>

<p class=MsoNormal>Contributions are welcome! To contribute:</p>

<ul style='margin-top:0in' type=disc>
 <li class=MsoNormal style='mso-list:l6 level1 lfo6;tab-stops:list .5in'>Fork
     the repository: <span class=CodeChar><span style='font-size:10.0pt;
     mso-bidi-font-size:12.0pt;line-height:115%'>https://github.com/your-username/projanitor</span></span></li>
 <li class=MsoNormal style='mso-list:l6 level1 lfo6;tab-stops:list .5in'>Create
     a feature branch: <span class=GramE><span class=CodeChar><span
     style='font-size:10.0pt;mso-bidi-font-size:12.0pt;line-height:115%'>git</span></span></span><span
     class=CodeChar><span style='font-size:10.0pt;mso-bidi-font-size:12.0pt;
     line-height:115%'> checkout -b feature/my-feature</span></span></li>
 <li class=MsoNormal style='mso-list:l6 level1 lfo6;tab-stops:list .5in'>Commit
     changes: <span class=CodeChar><span style='font-size:10.0pt;mso-bidi-font-size:
     12.0pt;line-height:115%'>git commit -m &quot;Add my feature&quot;</span></span></li>
 <li class=MsoNormal style='mso-list:l6 level1 lfo6;tab-stops:list .5in'>Push
     to the branch: <span class=CodeChar><span style='font-size:10.0pt;
     mso-bidi-font-size:12.0pt;line-height:115%'>git push origin
     feature/my-feature</span></span></li>
 <li class=MsoNormal style='mso-list:l6 level1 lfo6;tab-stops:list .5in'>Open a
     pull request.</li>
</ul>

<p class=MsoNormal>See CONTRIBUTING.md for guidelines.</p>

<h1>Testing</h1>

<p class=MsoNormal>To test <span class=SpellE><b>projanitor</b></span>:</p>

<ul style='margin-top:0in' type=disc>
 <li class=MsoNormal style='mso-list:l2 level1 lfo7;tab-stops:list .5in'>Navigate
     to your project directory:</li>
</ul>

<p class=Code style='text-indent:.5in'>cd /home/tom/work/<span class=SpellE>ProjectA</span></p>

<ul style='margin-top:0in' type=disc>
 <li class=MsoNormal style='mso-list:l2 level1 lfo7;tab-stops:list .5in'>Run
     with verbose mode:</li>
</ul>

<p class=MsoNormal style='text-indent:.5in'><span class=CodeChar><span
style='font-size:10.0pt;mso-bidi-font-size:12.0pt;line-height:115%'>/home/tom/work/ProjectA/ProjectA_build_scripts/House_cleaning/projanitor.py
--verbose</span></span></p>

<ul style='margin-top:0in' type=disc>
 <li class=MsoNormal style='mso-list:l2 level1 lfo7;tab-stops:list .5in'>Create
     a test project with:</li>
 <ul style='margin-top:0in' type=disc>
  <ul style='margin-top:0in' type=disc>
   <li class=MsoNormal style='mso-list:l2 level3 lfo7;tab-stops:list 1.5in'>Duplicate
       files (e.g., <span class=SpellE>src</span>/<span class=SpellE>main.c</span>
       and backup/<span class=SpellE>main.c</span>).</li>
   <li class=MsoNormal style='mso-list:l2 level3 lfo7;tab-stops:list 1.5in'><span
       class=SpellE>Symlinks</span> (e.g., ln -s <span class=SpellE>src</span>/<span
       class=SpellE>main.c</span> <span class=SpellE>link.c</span>).</li>
   <li class=MsoNormal style='mso-list:l2 level3 lfo7;tab-stops:list 1.5in'>Unreadable
       files (e.g., <span class=SpellE>chmod</span> 000 <span class=SpellE>test<span
       class=GramE>.c</span></span>).</li>
  </ul>
 </ul>
 <li class=MsoNormal style='mso-list:l2 level1 lfo7;tab-stops:list .5in'>Verify
     the report for accurate detection of duplicates, orphans, and missing
     files.</li>
</ul>

<h1>License</h1>

<p class=MsoNormal>MIT License (LICENSE) - Copyright (c) 2025 [Y-F Daniel Cheng]
</p>

<h1>Contact</h1>

<p class=MsoNormal>For issues or feature requests, open an issue on the <a
href="https://github.com/dyfcheng/projanitor" target="_blank">GitHub repository</a>
</p>

