# Redaction Protection

Library that handles locating and fixing vulnerable trivial and nontrivial redactions inside PDF files.

## Installing 

`./install.sh` should handle making all the dependencies, though you may need to install libraries used by poppler.

## AWS Usecase

`aws_trivial_redaction_locate.rb` is a script which does a simple first pass of a PDF document using courtlistener's 
xray utility to identify whether a document has any black boxes overlaying text, and then uses `locate.sh` to perform
a more fine-grained analysis to detect trivial redactions.

## Nontrivial Redactions

The `nontrivial` directory contains scripts outlining how to protect nontrivial redactions.
