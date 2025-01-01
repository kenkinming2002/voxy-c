# libnet
Implementation of a non-blocking message-oriented server client networking
library.

## TLS compression
We want to take advantage of TLS compression in order to reduce network
traffic.

However, TLS compression is disabled by default so as to mitigate the CRIME
vulnerability when TLS is used in the context of HTTPS. In our case, both the
server and client are trusted and we are in control of what data we sent.
Hence, enabling TLS compression does not pose any security risk.

Unfortunately, because persumably everything is about web, and our use cases is
supposedly non-standard, we have to jump through hoops in order to enable TLS
compression.

First, it is important that we compile OpenSSL with compression support.
Unfortunately, because TLS compression is disabled by default, it is likely
that the version of OpenSSL shipped with your distro is not compiled with
OpenSSL support. In that case, you would have compile OpenSSL version from
source by yourself. If you fail to do so, the library will still work, but
there will be no TLS compression.

If all you need is to get TLS compression working with libnet, you could stop
reading right here.

We also need to limit ourselves to TLS v1.2 because compression is strictly
prohibited in TLS v1.3. This is done by calling
`SSL_CTX_set_max_proto_version(ssl_ctx, TLS1_2_VERSION)`. Theoretically, there
is nothing in the on-the-wire format that prohibits compression in TLS v1.3
since it is somewhat backward compatible with TLS v1.2. Practically, at least
OpenSSL does not support compression with TLS v1.3.

Lastly, we also need to tell OpenSSL that we are okay with TLS compression.
This is done by calling `SSL_CTX_set_security_level(ssl_ctx, 1)` and
`SSL_CTX_clear_options(ssl_ctx, SSL_OP_NO_COMPRESSION)`.
