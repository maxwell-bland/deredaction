## Nontrivial Redaction Protections

Simple reference design for protecting nontrivially redacted documents.
Uses the pts intemediate representation to extract all character values in the document and 
rewrite the pdf in a monospace font. 

TODO: scaling of the monospace font so that the original line bounding box is preserved, and 
potentially unicode handling.

# Usage:

`./nontrivial_redaction_fix.sh testcases/trivial_1.redacted.pdf testcases/trivial_1.nontrivial.redacted.pdf`
