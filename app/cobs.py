class COBSError(Exception):
    pass

class ZeroError(COBSError):
    pass

class FormatError(COBSError):
    pass
    
def encode(decoded):
    chunk = bytes()
    encoded = bytes()
    for b in decoded:
        if b == 0:
            encoded += bytes([len(chunk)+1]) + chunk
            chunk = bytes()
        else:
            chunk += bytes([b])
            if len(chunk) == 254:
                encoded += bytes([len(chunk)+1]) + chunk
                chunk = bytes()
    return encoded + bytes([len(chunk)+1]) + chunk

def decode(encoded):
    if 0 in encoded:
        raise ZeroError('Zero byte encountered in encoded packet {}'.format(encoded))
    chunk_next = 0
    decoded = bytes()
    while chunk_next < len(encoded):
        chunk_start = chunk_next
        chunk_next += encoded[chunk_next]
        if chunk_next > len(encoded):
            raise FormatError('Incorrect format detected in packet {}'.format(encoded))
        if chunk_next == len(encoded):
            decoded += encoded[chunk_start+1:chunk_next] # Last chunk, remove phantom 0
        else:
            decoded += encoded[chunk_start+1:chunk_next] + bytes([0])
    return decoded
