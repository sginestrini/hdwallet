#if DEBUG_LINK

void fsm_msgDebugLinkGetState(DebugLinkGetState *msg)
{
	(void)msg;

	// Do not use RESP_INIT because it clears msg_resp, but another message
	// might be being handled
	DebugLinkState resp;
	memset(&resp, 0, sizeof(resp));

	resp.has_layout = true;
	resp.layout.size = OLED_BUFSIZE;
	memcpy(resp.layout.bytes, oledGetBuffer(), OLED_BUFSIZE);

	if (storage_hasPin()) {
		resp.has_pin = true;
		strlcpy(resp.pin, storage_getPin(), sizeof(resp.pin));
	}

	resp.has_matrix = true;
	strlcpy(resp.matrix, pinmatrix_get(), sizeof(resp.matrix));

	resp.has_reset_entropy = true;
	resp.reset_entropy.size = reset_get_int_entropy(resp.reset_entropy.bytes);

	resp.has_reset_word = true;
	strlcpy(resp.reset_word, reset_get_word(), sizeof(resp.reset_word));

	resp.has_recovery_fake_word = true;
	strlcpy(resp.recovery_fake_word, recovery_get_fake_word(), sizeof(resp.recovery_fake_word));

	resp.has_recovery_word_pos = true;
	resp.recovery_word_pos = recovery_get_word_pos();

	if (storage_hasMnemonic()) {
		resp.has_mnemonic = true;
		strlcpy(resp.mnemonic, storage_getMnemonic(), sizeof(resp.mnemonic));
	}

	if (storage_hasNode()) {
		resp.has_node = true;
		storage_dumpNode(&(resp.node));
	}

	resp.has_passphrase_protection = true;
	resp.passphrase_protection = storage_hasPassphraseProtection();

	msg_debug_write(MessageType_MessageType_DebugLinkState, &resp);
}

void fsm_msgDebugLinkStop(DebugLinkStop *msg)
{
	(void)msg;
}

void fsm_msgDebugLinkMemoryRead(DebugLinkMemoryRead *msg)
{
	RESP_INIT(DebugLinkMemory);

	uint32_t length = 1024;
	if (msg->has_length && msg->length < length)
		length = msg->length;
	resp->has_memory = true;
	memcpy(resp->memory.bytes, FLASH_PTR(msg->address), length);
	resp->memory.size = length;
	msg_debug_write(MessageType_MessageType_DebugLinkMemory, resp);
}

void fsm_msgDebugLinkMemoryWrite(DebugLinkMemoryWrite *msg)
{
	uint32_t length = msg->memory.size;
	if (msg->flash) {
		svc_flash_unlock();
		svc_flash_program(FLASH_CR_PROGRAM_X32);
		for (uint32_t i = 0; i < length; i += 4) {
			uint32_t word;
			memcpy(&word, msg->memory.bytes + i, 4);
			flash_write32(msg->address + i, word);
		}
		uint32_t dummy = svc_flash_lock();
		(void)dummy;
	} else {
#if !EMULATOR
		memcpy((void *) msg->address, msg->memory.bytes, length);
#endif
	}
}

void fsm_msgDebugLinkFlashErase(DebugLinkFlashErase *msg)
{
	svc_flash_unlock();
	svc_flash_erase_sector(msg->sector);
	uint32_t dummy = svc_flash_lock();
	(void)dummy;
}
#endif
