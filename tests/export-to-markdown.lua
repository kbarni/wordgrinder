--!nonstrict
loadfile("tests/testsuite.lua")()

Cmd.InsertStringIntoParagraph("one two three")
Cmd.SplitCurrentParagraph()

Cmd.InsertStringIntoParagraph("four")
Cmd.SplitCurrentWord()
Cmd.SetMark()
Cmd.InsertStringIntoParagraph("bold")
Cmd.SetStyle("b")
Cmd.SetMark()
Cmd.InsertStringIntoParagraph("italic")
Cmd.SetStyle("i")
Cmd.SetMark()
Cmd.InsertStringIntoParagraph("underline")
Cmd.SplitCurrentWord()
Cmd.InsertStringIntoParagraph("stillunderline")
Cmd.SetStyle("u")
Cmd.SetStyle("o")
Cmd.InsertStringIntoParagraph("plain")
Cmd.SplitCurrentParagraph()

Cmd.InsertStringIntoParagraph("heading")
Cmd.ChangeParagraphStyle("H1")
Cmd.SplitCurrentParagraph()

Cmd.InsertStringIntoParagraph("bullet")
Cmd.ChangeParagraphStyle("LB")
Cmd.SplitCurrentParagraph()

Cmd.InsertStringIntoParagraph("no bullet")
Cmd.ChangeParagraphStyle("L")
Cmd.SplitCurrentParagraph()

Cmd.InsertStringIntoParagraph("numbered")
Cmd.ChangeParagraphStyle("LN")
Cmd.SplitCurrentParagraph()

Cmd.InsertStringIntoParagraph("normal text again")
Cmd.ChangeParagraphStyle("P")

local expected = [[

one two three

four <b>bold</b><i><b>italic</b></i><i><b><u>underline </u></b></i><i><b><u>stillunderline</u></b></i>plain

# heading


- bullet
- no bullet
1. numbered

normal text again

]]

local output = Cmd.ExportToMarkdownString()
AssertEquals(expected, output)

-- Regression test for issue #316: plain text that looks like a Markdown block
-- marker must round-trip through .md export and import unchanged. Without
-- escaping, cmark re-reads "---" as a thematic break (and the paragraph
-- vanishes entirely), "1." as an ordered-list item, and "+" as a bullet.

ResetDocumentSet()
Cmd.InsertStringIntoParagraph("Chapter ends here.")
Cmd.SplitCurrentParagraph()
Cmd.InsertStringIntoParagraph("---")
Cmd.SplitCurrentParagraph()
Cmd.InsertStringIntoParagraph("New scene begins.")
Cmd.SplitCurrentParagraph()
Cmd.InsertStringIntoParagraph("1. First reason")
Cmd.SplitCurrentParagraph()
Cmd.InsertStringIntoParagraph("+ plus bullet")
Cmd.ChangeParagraphStyle("P")
local original_text = Cmd.ExportToTextString()

-- The block markers must be escaped in the exported markdown.
local roundtrip = Cmd.ExportToMarkdownString()
AssertEquals(true, roundtrip:find("\\---", 1, true) ~= nil)
AssertEquals(true, roundtrip:find("1\\.", 1, true) ~= nil)
AssertEquals(true, roundtrip:find("\\+", 1, true) ~= nil)

-- And re-importing the export must reproduce the original plain text, with the
-- "---" paragraph still present and the list-like lines still ordinary
-- paragraphs.
-- (cmark always prepends an empty leading paragraph on import, so compare
-- against the original text with a matching leading blank line.)
local reimported = Cmd.ImportMarkdownString(roundtrip)
documentSet:addDocument(reimported, "reimported")
documentSet:setCurrent("reimported")
local roundtripped_text = Cmd.ExportToTextString()
AssertEquals("\n" .. original_text, roundtripped_text)

