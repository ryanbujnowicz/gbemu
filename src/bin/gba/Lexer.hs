module Lexer 
( Token(..)
, scan
) where

import Data.Char

data Token = NameToken { name :: String }
           | NumberToken { num :: Int }
           | OpenParenthesisToken
           | CloseParenthesisToken
           | PlusToken
           | CommaToken
           | EndStatementToken
           deriving (Show)

-- TODO: readMaybe

isValidChar :: Char -> Bool
isValidChar c = c `elem` validChars
    where validChars = ['a'..'z'] ++ ['A'..'Z']

isValidNum :: Char -> Bool
isValidNum c = c `elem` validNums
    where validNums = ['0'..'9'] ++ ['x', 'X'] ++ ['a'..'z'] ++ ['A'..'Z']

spanName :: String -> [Token]
spanName (c:cs) = NameToken nameUp:scan rest
    where (name,rest) = span isValidChar (c:cs)
          nameUp = map toUpper name

spanNum :: String -> [Token]
spanNum (c:cs) = NumberToken num:scan rest
    where (numS,rest) = span isValidNum (c:cs)
          num = read numS :: Int 

scan :: String -> [Token]
scan "" = [EndStatementToken]
scan ('\n':cs) = EndStatementToken:scan cs
scan ('(':cs) = OpenParenthesisToken:scan cs
scan (')':cs) = CloseParenthesisToken:scan cs
scan ('+':cs) = PlusToken:scan cs
scan (',':cs) = CommaToken:scan cs
scan (' ':cs) = scan cs
scan all@('0':'x':cs) = spanNum all
scan (c:cs)
    | isValidChar c = spanName (c:cs)
    | isValidNum c = spanNum (c:cs)

