import numpy as np

class signal(object):
    """
    Signal class generates and stores different type of signal. 
    """
    __std_wavefroms = ["DC", "gaussian", "sinegaussian","sine", "chirp", 
                        "sawtooth", "square", "sweep_poly"]
    def __init__(self,waveform, fs, timelength, name = None, 
                timeofarrival = None, mood = None, **kwargs):

                 
        self.name = name
        self.fs = fs
        self.timelength = timelength
        self.TOA = timeofarrival
        self.waveForm = waveform
        self.signalArray = []
        for (key, value) in kwargs.items():
                        setattr(self, key, value)
        self.timeAxis = np.linspace(0, self.timelength, self.timelength*self.fs)
    def sig_gen(self):
        try:
            if(self.waveForm == ''):
                self.waveForm = "DC"
                for timePoint in self.timeAxis:
                    self.signalArray.append(1.0)
            elif(self.waveForm.lower() == "gaussian"):
                pass
            elif(self.waveForm.lower() == "sinegaussian"):
                pass
            elif(self.waveForm.lower() == "chirp"):
                pass
            elif(self.waveForm.lower() == "sawtooth"):
                pass
            elif(self.waveForm.lower() == "square"):    
            self.signalArray = np.array(self.signalArray) 
    def add_waveform(self):
        pass

class data(object):
	"""
	dataTS class creats, stores and describes simulated time seris data and it's
    parameters.
	signal is the signal wave form with out noise. The wave form can be selected
    and constructed by function "signal_create"
	noise is the noise with out signal. Noise type can be selected or created by
    function "nise_create"
	"""
	def __init__(self, name=None, fs = None, datatimelength = None,
                signaltimelength = None,waveform =[] ,noiseform = [],
                data=[],signal=[],noise = []):
		self.name = name
		self.fs = fs
		self.dataTleght = datatimelegth  # data time lenght
		self.signalTlenght = None
		self.waveForm = waveform
		self.noiseFrom = noiseform
		self.data = data   # data array for storing data
		self.signal = signal  # signal array for storing signal without noise
		self.noise = noise  # noise array for storing noise with signal



        
		

