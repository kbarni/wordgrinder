--!strict
loadfile("tests/testsuite.lua")()

Cmd.InsertStringIntoParagraph("12345")
Cmd.SplitCurrentParagraph()
Cmd.InsertStringIntoParagraph("67890")
Cmd.GotoPreviousCharW()

-- GotoPrevious/NextLine won't work without this.
Document:wrap(80)

AssertEquals(2, Document.cp)
AssertEquals(5, Document.co)

Cmd.GotoPreviousLine()

AssertEquals(1, Document.cp)
AssertEquals(5, Document.co)

