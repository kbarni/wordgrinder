loadfile("tests/testsuite.lua")()

data = [[
<html xmlns="http://www.w3.org/1999/xhtml"><head>
<meta http-equiv="Content-Type" content="text/html;charset=utf-8"/>
<meta name="generator" content="WordGrinder 0.8"/>
<title>main</title>
</head><body>

<p>one two three</p>
<p>four <b>bold<i>italic</i><u>underline</u></b></p>
<h1>heading</h1>
<ul>
<li>bullet</li>
<li style="list-style-type: none;">no bullet</li>
<li style="list-style-type: decimal;" value=1>numbered</li>
</ul>
<p>normal text again</p>
<ol>
<li>number</li>
</ol>
</body>
</html>
]]

local document = Cmd.ImportHTMLData(data)

local expected = [[
%% This document automatically generated by WordGrinder @@@.
\documentclass{article}
\usepackage{xunicode, setspace, xltxtra}
\sloppy
\onehalfspacing
\begin{document}
\title{imported}
\author{(no author)}
\maketitle
one two three

four \textbf{bold}\textit{\textbf{italic}}\textbf{\underline{underline}}

\section{heading}
\begin{enumerate}
\item[\textbullet]{bullet}
\item[\textbullet]{no bullet}
\item[\textbullet]{numbered}
\end{enumerate}
normal text again

\begin{enumerate}
\item{number}
\end{enumerate}
\end{document}
]]
expected = expected:gsub("@@@", VERSION)

DocumentSet:addDocument(document, "imported")
DocumentSet:setCurrent("imported")
local output = Cmd.ExportToLatexString()
AssertEquals(expected, output)


