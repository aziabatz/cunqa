

class Result():
    def __init__(self, result):
        if type(result) == dict:
            self.result = result
        else:
            print("Result format not supported, must be dict or list.")
            return

        for k,v in result.items():
            if k == "metadata":
                for i, m in v.items():
                    setattr(self, i, m)
            elif k == "results":
                for i, m in v[0].items():
                    if i == "data":
                        counts = m["counts"]
                    elif i == "metadata":
                        for j, w in m.items():
                            setattr(self,j,w)
                    else:
                        setattr(self, i, m)
            else:
                setattr(self, k, v)

        self.counts = {}
        for j,w in counts.items():
            self.counts[format( int(j, 16), '0'+str(self.num_qubits)+'b' )]= w
        
    def get_dict(self):
        return self.result

    def get_counts(self):
        return self.counts
