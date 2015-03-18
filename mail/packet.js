function Packet(src, dst, seqNumber, cNumber, label, data) {
    this.src = src;
    this.dst = dst;
    this.seqNumber = seqNumber;
    this.cNumber = cNumber;
    this.label = label;
    this.data = data;
}

exports.Packet = Packet;
