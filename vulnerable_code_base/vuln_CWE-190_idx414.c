    StreamBufferHandle_t xStreamBufferGenericCreate( size_t xBufferSizeBytes,
                                                     size_t xTriggerLevelBytes,
                                                     BaseType_t xIsMessageBuffer )
    {
        uint8_t * pucAllocatedMemory;
        uint8_t ucFlags;
        if( xIsMessageBuffer == pdTRUE )
        {
            ucFlags = sbFLAGS_IS_MESSAGE_BUFFER;
            configASSERT( xBufferSizeBytes > sbBYTES_TO_STORE_MESSAGE_LENGTH );
        }
        else
        {
            ucFlags = 0;
            configASSERT( xBufferSizeBytes > 0 );
        }
        configASSERT( xTriggerLevelBytes <= xBufferSizeBytes );
        if( xTriggerLevelBytes == ( size_t ) 0 )
        {
            xTriggerLevelBytes = ( size_t ) 1;
        }
        xBufferSizeBytes++;
        pucAllocatedMemory = ( uint8_t * ) pvPortMalloc( xBufferSizeBytes + sizeof( StreamBuffer_t ) );  
        if( pucAllocatedMemory != NULL )
        {
            prvInitialiseNewStreamBuffer( ( StreamBuffer_t * ) pucAllocatedMemory,            
                                          pucAllocatedMemory + sizeof( StreamBuffer_t ),    
                                          xBufferSizeBytes,
                                          xTriggerLevelBytes,
                                          ucFlags );
            traceSTREAM_BUFFER_CREATE( ( ( StreamBuffer_t * ) pucAllocatedMemory ), xIsMessageBuffer );
        }
        else
        {
            traceSTREAM_BUFFER_CREATE_FAILED( xIsMessageBuffer );
        }
        return ( StreamBufferHandle_t ) pucAllocatedMemory;  
    }
