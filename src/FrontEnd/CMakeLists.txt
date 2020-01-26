set(_SOURCE_FILES
  Lexer.cpp
  AST.cpp
  ASTConstructor.cpp
  )

add_library(FrontEnd OBJECT
  ${_SOURCE_FILES})

# TODO: Add code for unit tests
