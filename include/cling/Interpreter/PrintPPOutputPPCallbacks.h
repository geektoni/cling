//
// Created by uriel on 14/07/16.
//

#ifndef CLANG_TEST_PRINTPPOUTPUTPPCALLBACKS_H
#define CLANG_TEST_PRINTPPOUTPUTPPCALLBACKS_H

#include "clang/Frontend/Utils.h"
#include "clang/Basic/CharInfo.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/PreprocessorOutputOptions.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Pragma.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/TokenConcatenation.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "PrintPPOutputPPCallbacks.h"

using namespace clang;

class PrintPPOutputPPCallbacks : public PPCallbacks {
    Preprocessor &PP;
    SourceManager &SM;
    TokenConcatenation ConcatInfo;
    public:
        raw_ostream &OS;
    private:
        unsigned CurLine;

        bool EmittedTokensOnThisLine;
        bool EmittedDirectiveOnThisLine;
        SrcMgr::CharacteristicKind FileType;
        SmallString<512> CurFilename;
        bool Initialized;
        bool DisableLineMarkers;
        bool DumpDefines;
        bool UseLineDirective;
        bool IsFirstFileEntered;
    public:
        PrintPPOutputPPCallbacks(Preprocessor &pp, raw_ostream &os,
                                 bool lineMarkers, bool defines)
                : PP(pp), SM(PP.getSourceManager()),
                  ConcatInfo(PP), OS(os), DisableLineMarkers(lineMarkers),
                  DumpDefines(defines) {
            CurLine = 0;
            CurFilename += "<uninit>";
            EmittedTokensOnThisLine = false;
            EmittedDirectiveOnThisLine = false;
            FileType = SrcMgr::C_User;
            Initialized = false;
            IsFirstFileEntered = false;

            // If we're in microsoft mode, use normal #line instead of line markers.
            UseLineDirective = PP.getLangOpts().MicrosoftExt;
        }

        void setEmittedTokensOnThisLine() { EmittedTokensOnThisLine = true; }
        bool hasEmittedTokensOnThisLine() const { return EmittedTokensOnThisLine; }

        void setEmittedDirectiveOnThisLine() { EmittedDirectiveOnThisLine = true; }
        bool hasEmittedDirectiveOnThisLine() const {
            return EmittedDirectiveOnThisLine;
        }

        bool startNewLineIfNeeded(bool ShouldUpdateCurrentLine = true);

        void FileChanged(SourceLocation Loc, FileChangeReason Reason,
                         SrcMgr::CharacteristicKind FileType,
                         FileID PrevFID) override;
        void InclusionDirective(SourceLocation HashLoc, const Token &IncludeTok,
                                StringRef FileName, bool IsAngled,
                                CharSourceRange FilenameRange, const FileEntry *File,
                                StringRef SearchPath, StringRef RelativePath,
                                const Module *Imported) override;
        void Ident(SourceLocation Loc, const std::string &str) override;
        void PragmaMessage(SourceLocation Loc, StringRef Namespace,
                           PragmaMessageKind Kind, StringRef Str) override;
        void PragmaDebug(SourceLocation Loc, StringRef DebugType) override;
        void PragmaDiagnosticPush(SourceLocation Loc, StringRef Namespace) override;
        void PragmaDiagnosticPop(SourceLocation Loc, StringRef Namespace) override;
        void PragmaDiagnostic(SourceLocation Loc, StringRef Namespace,
                              diag::Severity Map, StringRef Str) override;
        void PragmaWarning(SourceLocation Loc, StringRef WarningSpec,
                           ArrayRef<int> Ids) override;
        void PragmaWarningPush(SourceLocation Loc, int Level) override;
        void PragmaWarningPop(SourceLocation Loc) override;

        bool HandleFirstTokOnLine(Token &Tok);

        /// Move to the line of the provided source location. This will
        /// return true if the output stream required adjustment or if
        /// the requested location is on the first line.
        bool MoveToLine(SourceLocation Loc) {
            PresumedLoc PLoc = SM.getPresumedLoc(Loc);
            if (PLoc.isInvalid())
                return false;
            return MoveToLine(PLoc.getLine()) || (PLoc.getLine() == 1);
        }
        bool MoveToLine(unsigned LineNo);

        bool AvoidConcat(const Token &PrevPrevTok, const Token &PrevTok,
                         const Token &Tok) {
            return ConcatInfo.AvoidConcat(PrevPrevTok, PrevTok, Tok);
        }
        void WriteLineInfo(unsigned LineNo, const char *Extra=nullptr,
                           unsigned ExtraLen=0);
        bool LineMarkersAreDisabled() const { return DisableLineMarkers; }
        void HandleNewlinesInToken(const char *TokStr, unsigned Len);

        /// MacroDefined - This hook is called whenever a macro definition is seen.
        void MacroDefined(const Token &MacroNameTok,
                          const MacroDirective *MD) override;

        // Commented because there is no MacroDefinition in this version of Clang.
        /// MacroUndefined - This hook is called whenever a macro #undef is seen.
        //void MacroUndefined(const Token &MacroNameTok,
        //                   const MacroDefinition &MD) override;
};

struct UnknownPragmaHandler : public PragmaHandler {
    const char *Prefix;
    PrintPPOutputPPCallbacks *Callbacks;

    UnknownPragmaHandler(const char *prefix, PrintPPOutputPPCallbacks *callbacks)
            : Prefix(prefix), Callbacks(callbacks) {}
    void HandlePragma(Preprocessor &PP, PragmaIntroducerKind Introducer,
                      Token &PragmaTok) override {
        // Figure out what line we went to and insert the appropriate number of
        // newline characters.
        Callbacks->startNewLineIfNeeded();
        Callbacks->MoveToLine(PragmaTok.getLocation());
        Callbacks->OS.write(Prefix, strlen(Prefix));
        // Read and print all of the pragma tokens.
        while (PragmaTok.isNot(tok::eod)) {
            if (PragmaTok.hasLeadingSpace())
                Callbacks->OS << ' ';
            std::string TokSpell = PP.getSpelling(PragmaTok);
            Callbacks->OS.write(&TokSpell[0], TokSpell.size());

            // Expand macros in pragmas with -fms-extensions.  The assumption is that
            // the majority of pragmas in such a file will be Microsoft pragmas.
            if (PP.getLangOpts().MicrosoftExt)
                PP.Lex(PragmaTok);
            else
                PP.LexUnexpandedToken(PragmaTok);
        }
        Callbacks->setEmittedDirectiveOnThisLine();
    }
};

void PrintMacroDefinition(const IdentifierInfo &II, const MacroInfo &MI,
                                 Preprocessor &PP, raw_ostream &OS);

static void PrintPreprocessedTokens(Preprocessor &PP, Token &Tok,
                                    PrintPPOutputPPCallbacks *Callbacks,
                                    raw_ostream &OS);

static void DoPrintMacros(Preprocessor &PP, raw_ostream *OS);

void clang::DoPrintPreprocessedInput(Preprocessor &PP, raw_ostream *OS,
                                     const PreprocessorOutputOptions &Opts);


#endif //CLANG_TEST_PRINTPPOUTPUTPPCALLBACKS_H
