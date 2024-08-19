import React, { useState } from 'react';
import useWasmModule from './useWasmModule';
import './App.css'; // Assurez-vous de créer ce fichier CSS

function App() {
  const [sequences, setSequences] = useState({ seq1: "", seq2: "" });
  const [alignment, setAlignment] = useState({ c1: "", c2: "" });
  const [similarityInfo, setSimilarityInfo] = useState({ max: 0, count: 0 });

  const { module: wasmModule, error, isLoading } = useWasmModule('/common_molecular.js');

  const handleFileUpload = (event) => {
    const file = event.target.files[0];
    if (file) {
      const reader = new FileReader();
      reader.onload = (e) => {
        const content = e.target.result;
        const [seq1, seq2] = content.split('\n').map(seq => seq.trim());
        setSequences({ seq1, seq2 });
      };
      reader.readAsText(file);
    }
  };

  const alignSequences = () => {
    if (!wasmModule || typeof wasmModule.ccall !== 'function') return;

    const { seq1, seq2 } = sequences;
    
    try {
      const resultPtr = wasmModule.ccall(
        'align_sequences',
        'number',
        ['string', 'string'],
        [seq1, seq2]
      );

      if (resultPtr === 0) {
        console.error("Alignment function returned null pointer");
        return;
      }

      const c1 = wasmModule.UTF8ToString(wasmModule.getValue(resultPtr, '*'));
      const c2 = wasmModule.UTF8ToString(wasmModule.getValue(resultPtr + 4, '*'));
      const similarity = wasmModule.getValue(resultPtr + 8, 'double');
      const occurrences = wasmModule.getValue(resultPtr + 16, 'i32');

      setAlignment({ c1, c2 });
      setSimilarityInfo({ max: similarity, count: occurrences });

      wasmModule.ccall('free_alignment_result', null, ['number'], [resultPtr]);
    } catch (err) {
      console.error("Error during alignment:", err);
    }
  };

  if (isLoading) return <div className="loading">Loading WebAssembly module...</div>;
  if (error) return <div className="error">Error loading WebAssembly module: {error.message}</div>;

  return (
    <div className="App">
      <h1>🧬 GeneAlign Pro: Sequence Harmony Explorer</h1>
      <p className="subtitle">
        An implementation of the Smith-Waterman algorithm for identifying common molecular subsequences, 
        based on the 1981 paper by T.F. SMITH and M.S. WATERMAN.
      </p>
      <div className="input-section">
        <div className="file-input">
          <label htmlFor="file-upload" className="custom-file-upload">
            Upload Sequence File
          </label>
          <input id="file-upload" type="file" accept=".txt" onChange={handleFileUpload} />
        </div>
        <div className="sequence-input">
          <input
            type="text"
            value={sequences.seq1}
            onChange={(e) => setSequences({ ...sequences, seq1: e.target.value })}
            placeholder="Enter Sequence 1"
          />
          <input
            type="text"
            value={sequences.seq2}
            onChange={(e) => setSequences({ ...sequences, seq2: e.target.value })}
            placeholder="Enter Sequence 2"
          />
        </div>
        <button onClick={alignSequences} className="align-button">Align Sequences</button>
      </div>

      <div className="results-section">
        <h2>Alignment Results</h2>
        <p>Maximal similarity degree: {similarityInfo.max.toFixed(2)}</p>
        <p>Occurrences: {similarityInfo.count}</p>
        <div className="alignment-display">
          <pre>{alignment.c1}</pre>
          <pre>{alignment.c2}</pre>
        </div>
      </div>
    </div>
  );
}

export default App;