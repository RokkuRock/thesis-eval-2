static UINT serial_process_irp_create(SERIAL_DEVICE* serial, IRP* irp)
{
	DWORD DesiredAccess;
	DWORD SharedAccess;
	DWORD CreateDisposition;
	UINT32 PathLength;
	if (Stream_GetRemainingLength(irp->input) < 32)
		return ERROR_INVALID_DATA;
	Stream_Read_UINT32(irp->input, DesiredAccess);      
	Stream_Seek_UINT64(irp->input);                     
	Stream_Seek_UINT32(irp->input);                     
	Stream_Read_UINT32(irp->input, SharedAccess);       
	Stream_Read_UINT32(irp->input, CreateDisposition);  
	Stream_Seek_UINT32(irp->input);                     
	Stream_Read_UINT32(irp->input, PathLength);         
	if (Stream_GetRemainingLength(irp->input) < PathLength)
		return ERROR_INVALID_DATA;
	Stream_Seek(irp->input, PathLength);  
	assert(PathLength == 0);              
#ifndef _WIN32
	WLog_Print(serial->log, WLOG_DEBUG,
	           "DesiredAccess: 0x%" PRIX32 ", SharedAccess: 0x%" PRIX32
	           ", CreateDisposition: 0x%" PRIX32 "",
	           DesiredAccess, SharedAccess, CreateDisposition);
	DesiredAccess = GENERIC_READ | GENERIC_WRITE;
	SharedAccess = 0;
	CreateDisposition = OPEN_EXISTING;
#endif
	serial->hComm =
	    CreateFile(serial->device.name, DesiredAccess, SharedAccess, NULL,  
	               CreateDisposition, 0,                                    
	               NULL);                                                   
	if (!serial->hComm || (serial->hComm == INVALID_HANDLE_VALUE))
	{
		WLog_Print(serial->log, WLOG_WARN, "CreateFile failure: %s last-error: 0x%08" PRIX32 "",
		           serial->device.name, GetLastError());
		irp->IoStatus = STATUS_UNSUCCESSFUL;
		goto error_handle;
	}
	_comm_setServerSerialDriver(serial->hComm, serial->ServerSerialDriverId);
	_comm_set_permissive(serial->hComm, serial->permissive);
	assert(irp->FileId == 0);
	irp->FileId = irp->devman->id_sequence++;  
	irp->IoStatus = STATUS_SUCCESS;
	WLog_Print(serial->log, WLOG_DEBUG, "%s (DeviceId: %" PRIu32 ", FileId: %" PRIu32 ") created.",
	           serial->device.name, irp->device->id, irp->FileId);
error_handle:
	Stream_Write_UINT32(irp->output, irp->FileId);  
	Stream_Write_UINT8(irp->output, 0);             
	return CHANNEL_RC_OK;
}