module Parser where

import Lexer
import Data.Char
import Data.Monoid
import Text.Read (readMaybe)

data Opcode = ADC | ADD | AND | BIT | CALL | CCF | CP | CPL | DAA | DEC | DI | EI | HALT | INC | JP | JR | LD | LDD | LDI | NOP | OR | POP | PUSH | RES | RET | RETI | RL | RLA | RLC | RLCA | RR | RRA | RRC | RRCA | RST | SBC | SCF | SET | SLA | SRA | SRL | STOP | SUB | SWAP | XOR
    deriving (Show, Read)

data Register = A | AF | B | BC | C | D | DE | E | H | HL | L | PC | SP 
    deriving (Show, Read)

data Flag = FlagZ | FlagNZ | FlagC | FlagNC
    deriving (Show, Read)

data ParseNode = NullaryOpNode { opcode :: Opcode }
               | UnaryOpNode { opcode :: Opcode , arg :: ParseNode }
               | BinaryOpNode { opcode :: Opcode, arg1 :: ParseNode, arg2 :: ParseNode }
               | RegNode { register :: Register }
               | FF00OffsetRegNode { register :: Register } -- offset from 0xFF00
               | FlagNode { flag :: Flag }
               | NumNode { number :: Int }
               | FF00OffsetNode { number :: Int } -- offset from 0xFF00
               | SPOffsetNode { number :: Int } -- offset from 0xFF00
               | MemNode { address :: Int }
               | MemRegNode { register :: Register }
               deriving (Show)

makeOpcode :: String -> Maybe Opcode
makeOpcode = readMaybe

makeRegister :: String -> Maybe Register
makeRegister cs = readMaybe cs

makeFlag :: String -> Maybe Flag
makeFlag cs = readMaybe ("Flag" ++ cs)

-- TODO: maybe [Token]
-- TODO: refactor fmap
-- TODO: make pattern matching smaller (DSL) possibly by aliases?
-- TODO: NumberToken -> NumToken
parse :: [Token] -> Maybe [ParseNode]
parse (NameToken op:EndStatementToken:ts) = do
    opcode <- makeOpcode op
    fmap ([NullaryOpNode opcode]++) $ parse ts
parse (NameToken op:NameToken reg:EndStatementToken:ts) = do
    opcode <- makeOpcode op
    register <- makeRegister reg
    fmap ([UnaryOpNode opcode (RegNode register)]++) $ parse ts
parse (NameToken op:OpenParenthesisToken:NameToken reg:CloseParenthesisToken:EndStatementToken:ts) = do
    opcode <- makeOpcode op
    register <- makeRegister reg
    fmap ([UnaryOpNode opcode (MemRegNode register)]++) $ parse ts
parse (NameToken op:OpenParenthesisToken:NumberToken num:CloseParenthesisToken:EndStatementToken:ts) = do
    opcode <- makeOpcode op
    fmap ([UnaryOpNode opcode (MemNode num)]++) $ parse ts
parse (NameToken op:NumberToken n:EndStatementToken:ts) = do
    opcode <- makeOpcode op
    fmap ([UnaryOpNode opcode (NumNode n)]++) $ parse ts
parse (NameToken op:NameToken regA:CommaToken:NameToken regB:EndStatementToken:ts) = do
    opcode <- makeOpcode op
    registerA <- makeRegister regA
    registerB <- makeRegister regB
    fmap ([BinaryOpNode opcode (RegNode registerA) (RegNode registerB)]++) $ parse ts
parse (NameToken op:NameToken reg:CommaToken:NumberToken num:EndStatementToken:ts) = do
    opcode <- makeOpcode op
    register <- makeRegister reg
    fmap ([BinaryOpNode opcode (RegNode register) (NumNode num)]++) $ parse ts
parse (NameToken op:OpenParenthesisToken:NameToken regA:CloseParenthesisToken:CommaToken:NameToken regB:EndStatementToken:ts) = do
    opcode <- makeOpcode op
    registerA <- makeRegister regA
    registerB <- makeRegister regB
    fmap ([BinaryOpNode opcode (MemRegNode registerA) (RegNode registerB)]++) $ parse ts
parse (NameToken op:OpenParenthesisToken:NameToken reg:CloseParenthesisToken:CommaToken:NumberToken num:EndStatementToken:ts) = do
    opcode <- makeOpcode op
    register <- makeRegister reg
    fmap ([BinaryOpNode opcode (MemRegNode register) (NumNode num)]++) $ parse ts
parse (NameToken op:NameToken regA:CommaToken:OpenParenthesisToken:NameToken regB:CloseParenthesisToken:EndStatementToken:ts) = do
    opcode <- makeOpcode op
    registerA <- makeRegister regA
    registerB <- makeRegister regB
    fmap ([BinaryOpNode opcode (RegNode registerA) (MemRegNode registerB)]++) $ parse ts
parse (NameToken op:OpenParenthesisToken:NumberToken num:CloseParenthesisToken:CommaToken:NameToken reg:EndStatementToken:ts) = do
    opcode <- makeOpcode op
    register <- makeRegister reg
    fmap ([BinaryOpNode opcode (MemNode num) (RegNode register)]++) $ parse ts
parse (NameToken op:OpenParenthesisToken:NameToken "PC":PlusToken:NumberToken num:CloseParenthesisToken:EndStatementToken:ts) = do
    opcode <- makeOpcode op
    fmap ([UnaryOpNode opcode (NumNode num)]++) $ parse ts
parse (NameToken op:NameToken f:CommaToken:OpenParenthesisToken:NameToken "PC":PlusToken:NumberToken num:CloseParenthesisToken:EndStatementToken:ts) = do
    opcode <- makeOpcode op
    flag <- makeFlag f
    fmap ([BinaryOpNode opcode (FlagNode flag) (NumNode num)]++) $ parse ts
parse (NameToken op:OpenParenthesisToken:NumberToken 0xFF00:PlusToken:NumberToken n:CloseParenthesisToken:CommaToken:NameToken reg:EndStatementToken:ts) = do
    opcode <- makeOpcode op
    reg <- makeRegister reg
    fmap ([BinaryOpNode opcode (FF00OffsetNode n) (RegNode reg)]++) $ parse ts
parse (NameToken op:OpenParenthesisToken:NumberToken 0xFF00:PlusToken:NameToken regA:CloseParenthesisToken:CommaToken:NameToken regB:EndStatementToken:ts) = do
    opcode <- makeOpcode op
    registerA <- makeRegister regA
    registerB <- makeRegister regB
    fmap ([BinaryOpNode opcode (FF00OffsetRegNode registerA) (RegNode registerB)]++) $ parse ts
parse (NameToken op:NameToken reg:CommaToken:OpenParenthesisToken:NumberToken 0xFF00:PlusToken:NumberToken n:CloseParenthesisToken:EndStatementToken:ts) = do
    opcode <- makeOpcode op
    reg <- makeRegister reg
    fmap ([BinaryOpNode opcode (RegNode reg) (FF00OffsetNode n)]++) $ parse ts
parse (NameToken op:NameToken regA:OpenParenthesisToken:NumberToken 0xFF00:PlusToken:NameToken regB:CloseParenthesisToken:EndStatementToken:ts) = do
    opcode <- makeOpcode op
    registerA <- makeRegister regA
    registerB <- makeRegister regB
    fmap ([BinaryOpNode opcode (RegNode registerA) (FF00OffsetRegNode registerB)]++) $ parse ts
parse (NameToken op:NameToken reg:CommaToken:NameToken "SP":PlusToken:NumberToken dd:EndStatementToken:ts) = do
    opcode <- makeOpcode op
    register <- makeRegister reg
    fmap ([BinaryOpNode opcode (RegNode register) (SPOffsetNode dd)]++) $ parse ts
parse [] = Just []

