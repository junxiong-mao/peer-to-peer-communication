# Peer to peer communication

The system is composed of server, client and tracker.

Server records a list of torrent files. The client will select a torrent file to be downloaded. After resolving the torrent file, client will get a list of peers who possess the data files from the tracker, and also register itself at the tracker.

While a client is downloading, it is also the file uploader, enabling other downloader to download from this uploader.