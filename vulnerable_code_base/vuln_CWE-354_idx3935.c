void recovery_character(const char *character)
{
    if (!awaiting_character) {
        recovery_abort();
        fsm_sendFailure(FailureType_Failure_UnexpectedMessage, "Not in Recovery mode");
        layoutHome();
        return;
    }
    if (strlen(mnemonic) + 1 > MNEMONIC_BUF - 1) {
        recovery_abort();
        fsm_sendFailure(FailureType_Failure_UnexpectedMessage,
                        "Too many characters attempted during recovery");
        layoutHome();
        return;
    }
    char *pos = strchr(cipher, character[0]);
    if (character[0] != ' ' && pos == NULL) {
        recovery_abort();
        fsm_sendFailure(FailureType_Failure_SyntaxError, "Character must be from a to z");
        layoutHome();
        return;
    }
    static int uncyphered_word_count = 0;
    static bool definitely_using_cipher = false;
    static CONFIDENTIAL char coded_word[12];
    static CONFIDENTIAL char decoded_word[12];
    if (!mnemonic[0]) {
        uncyphered_word_count = 0;
        definitely_using_cipher = false;
        memzero(coded_word, sizeof(coded_word));
        memzero(decoded_word, sizeof(decoded_word));
    }
    char decoded_character[2] = " ";
    if (character[0] != ' ') {
        decoded_character[0] = english_alphabet[(int)(pos - cipher)];
        strlcat(coded_word, character, sizeof(coded_word));
        strlcat(decoded_word, decoded_character, sizeof(decoded_word));
        if (enforce_wordlist && 4 <= strlen(coded_word)) {
            bool maybe_not_using_cipher = attempt_auto_complete(coded_word);
            bool maybe_using_cipher = attempt_auto_complete(decoded_word);
            if (!maybe_not_using_cipher && maybe_using_cipher) {
                definitely_using_cipher = true;
            } else if (maybe_not_using_cipher && !definitely_using_cipher &&
                       MAX_UNCYPHERED_WORDS < uncyphered_word_count++) {
                recovery_abort();
                fsm_sendFailure(FailureType_Failure_SyntaxError,
                                "Words were not entered correctly. Make sure you are using the substition cipher.");
                layoutHome();
                return;
            }
        }
    } else {
        memzero(coded_word, sizeof(coded_word));
        memzero(decoded_word, sizeof(decoded_word));
    }
    strlcat(mnemonic, decoded_character, MNEMONIC_BUF);
    next_character();
}