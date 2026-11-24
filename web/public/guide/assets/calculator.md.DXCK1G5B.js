import{_ as s,o as n,c as t,ag as e}from"./chunks/framework.CoHhbRn3.js";const h=JSON.parse('{"title":"Calculator","description":"","frontmatter":{},"headers":[],"relativePath":"calculator.md","filePath":"calculator.md"}'),p={name:"calculator.md"};function d(l,a,o,c,i,r){return n(),t("div",null,[...a[0]||(a[0]=[e(`<h1 id="calculator" tabindex="-1">Calculator <a class="header-anchor" href="#calculator" aria-label="Permalink to &quot;Calculator&quot;">​</a></h1><p>LinNote&#39;s calculator evaluates expressions in real-time.</p><p><strong>Activate:</strong></p><div class="language- vp-adaptive-theme"><button title="Copy Code" class="copy"></button><span class="lang"></span><pre class="shiki shiki-themes github-light github-dark vp-code" tabindex="0"><code><span class="line"><span>calc</span></span></code></pre></div><h2 id="basic-usage" tabindex="-1">Basic Usage <a class="header-anchor" href="#basic-usage" aria-label="Permalink to &quot;Basic Usage&quot;">​</a></h2><p>Type any expression. Add <code>=</code> at the end to see the result:</p><div class="language- vp-adaptive-theme"><button title="Copy Code" class="copy"></button><span class="lang"></span><pre class="shiki shiki-themes github-light github-dark vp-code" tabindex="0"><code><span class="line"><span>2 + 2 =</span></span>
<span class="line"><span>// Result: 4</span></span>
<span class="line"><span></span></span>
<span class="line"><span>100 / 4 =</span></span>
<span class="line"><span>// Result: 25</span></span></code></pre></div><h2 id="operators" tabindex="-1">Operators <a class="header-anchor" href="#operators" aria-label="Permalink to &quot;Operators&quot;">​</a></h2><table tabindex="0"><thead><tr><th>Operator</th><th>Description</th><th>Example</th></tr></thead><tbody><tr><td><code>+</code></td><td>Addition</td><td><code>5 + 3 = 8</code></td></tr><tr><td><code>-</code></td><td>Subtraction</td><td><code>10 - 4 = 6</code></td></tr><tr><td><code>*</code></td><td>Multiplication</td><td><code>6 * 7 = 42</code></td></tr><tr><td><code>/</code></td><td>Division</td><td><code>15 / 3 = 5</code></td></tr><tr><td><code>^</code></td><td>Power</td><td><code>2^8 = 256</code></td></tr><tr><td><code>%</code></td><td>Modulo</td><td><code>17 % 5 = 2</code></td></tr><tr><td><code>()</code></td><td>Parentheses</td><td><code>(2 + 3) * 4 = 20</code></td></tr></tbody></table><h2 id="percentages" tabindex="-1">Percentages <a class="header-anchor" href="#percentages" aria-label="Permalink to &quot;Percentages&quot;">​</a></h2><div class="language- vp-adaptive-theme"><button title="Copy Code" class="copy"></button><span class="lang"></span><pre class="shiki shiki-themes github-light github-dark vp-code" tabindex="0"><code><span class="line"><span>100 + 18% =</span></span>
<span class="line"><span>// Result: 118</span></span>
<span class="line"><span></span></span>
<span class="line"><span>500 - 20% =</span></span>
<span class="line"><span>// Result: 400</span></span>
<span class="line"><span></span></span>
<span class="line"><span>what is 15% of 200 =</span></span>
<span class="line"><span>// Result: 30</span></span></code></pre></div><h2 id="variables" tabindex="-1">Variables <a class="header-anchor" href="#variables" aria-label="Permalink to &quot;Variables&quot;">​</a></h2><p>Store values for reuse:</p><div class="language- vp-adaptive-theme"><button title="Copy Code" class="copy"></button><span class="lang"></span><pre class="shiki shiki-themes github-light github-dark vp-code" tabindex="0"><code><span class="line"><span>price = 100</span></span>
<span class="line"><span>quantity = 5</span></span>
<span class="line"><span>discount = 10</span></span>
<span class="line"><span></span></span>
<span class="line"><span>subtotal = price * quantity =</span></span>
<span class="line"><span>// Result: 500</span></span>
<span class="line"><span></span></span>
<span class="line"><span>discount_amount = subtotal * discount / 100 =</span></span>
<span class="line"><span>// Result: 50</span></span>
<span class="line"><span></span></span>
<span class="line"><span>total = subtotal - discount_amount =</span></span>
<span class="line"><span>// Result: 450</span></span></code></pre></div><h2 id="functions" tabindex="-1">Functions <a class="header-anchor" href="#functions" aria-label="Permalink to &quot;Functions&quot;">​</a></h2><table tabindex="0"><thead><tr><th>Function</th><th>Description</th><th>Example</th></tr></thead><tbody><tr><td><code>sqrt(x)</code></td><td>Square root</td><td><code>sqrt(144) = 12</code></td></tr><tr><td><code>abs(x)</code></td><td>Absolute value</td><td><code>abs(-5) = 5</code></td></tr><tr><td><code>sin(x)</code></td><td>Sine (radians)</td><td><code>sin(0) = 0</code></td></tr><tr><td><code>cos(x)</code></td><td>Cosine</td><td><code>cos(0) = 1</code></td></tr><tr><td><code>tan(x)</code></td><td>Tangent</td><td><code>tan(0) = 0</code></td></tr><tr><td><code>log(x)</code></td><td>Natural log</td><td><code>log(e) = 1</code></td></tr><tr><td><code>log10(x)</code></td><td>Log base 10</td><td><code>log10(100) = 2</code></td></tr><tr><td><code>floor(x)</code></td><td>Round down</td><td><code>floor(3.7) = 3</code></td></tr><tr><td><code>ceil(x)</code></td><td>Round up</td><td><code>ceil(3.2) = 4</code></td></tr><tr><td><code>round(x)</code></td><td>Round nearest</td><td><code>round(3.5) = 4</code></td></tr><tr><td><code>min(a,b)</code></td><td>Minimum</td><td><code>min(5, 3) = 3</code></td></tr><tr><td><code>max(a,b)</code></td><td>Maximum</td><td><code>max(5, 3) = 5</code></td></tr></tbody></table><h2 id="constants" tabindex="-1">Constants <a class="header-anchor" href="#constants" aria-label="Permalink to &quot;Constants&quot;">​</a></h2><table tabindex="0"><thead><tr><th>Constant</th><th>Value</th></tr></thead><tbody><tr><td><code>pi</code></td><td>3.14159265359</td></tr><tr><td><code>e</code></td><td>2.71828182846</td></tr></tbody></table><hr><h2 id="text-analysis-keywords" tabindex="-1">Text Analysis Keywords <a class="header-anchor" href="#text-analysis-keywords" aria-label="Permalink to &quot;Text Analysis Keywords&quot;">​</a></h2><p>Analyze numbers in your note:</p><h3 id="sum" tabindex="-1">sum <a class="header-anchor" href="#sum" aria-label="Permalink to &quot;sum&quot;">​</a></h3><p>Adds all numbers:</p><div class="language- vp-adaptive-theme"><button title="Copy Code" class="copy"></button><span class="lang"></span><pre class="shiki shiki-themes github-light github-dark vp-code" tabindex="0"><code><span class="line"><span>Groceries:</span></span>
<span class="line"><span>Milk 4.50</span></span>
<span class="line"><span>Bread 2.30</span></span>
<span class="line"><span>Eggs 6.00</span></span>
<span class="line"><span></span></span>
<span class="line"><span>sum</span></span></code></pre></div><p><strong>Result:</strong> <code>Sum: 12.80</code></p><h3 id="avg" tabindex="-1">avg <a class="header-anchor" href="#avg" aria-label="Permalink to &quot;avg&quot;">​</a></h3><p>Calculates average:</p><div class="language- vp-adaptive-theme"><button title="Copy Code" class="copy"></button><span class="lang"></span><pre class="shiki shiki-themes github-light github-dark vp-code" tabindex="0"><code><span class="line"><span>Test Scores:</span></span>
<span class="line"><span>85</span></span>
<span class="line"><span>92</span></span>
<span class="line"><span>78</span></span>
<span class="line"><span>88</span></span>
<span class="line"><span></span></span>
<span class="line"><span>avg</span></span></code></pre></div><p><strong>Result:</strong> <code>Average: 85.75</code></p><h3 id="count" tabindex="-1">count <a class="header-anchor" href="#count" aria-label="Permalink to &quot;count&quot;">​</a></h3><p>Counts numbers:</p><div class="language- vp-adaptive-theme"><button title="Copy Code" class="copy"></button><span class="lang"></span><pre class="shiki shiki-themes github-light github-dark vp-code" tabindex="0"><code><span class="line"><span>Items in list:</span></span>
<span class="line"><span>5 apples</span></span>
<span class="line"><span>3 bananas</span></span>
<span class="line"><span>8 oranges</span></span>
<span class="line"><span></span></span>
<span class="line"><span>count</span></span></code></pre></div><p><strong>Result:</strong> <code>Count: 3</code></p><hr><h2 id="practical-examples" tabindex="-1">Practical Examples <a class="header-anchor" href="#practical-examples" aria-label="Permalink to &quot;Practical Examples&quot;">​</a></h2><h3 id="tip-calculator" tabindex="-1">Tip Calculator <a class="header-anchor" href="#tip-calculator" aria-label="Permalink to &quot;Tip Calculator&quot;">​</a></h3><div class="language- vp-adaptive-theme"><button title="Copy Code" class="copy"></button><span class="lang"></span><pre class="shiki shiki-themes github-light github-dark vp-code" tabindex="0"><code><span class="line"><span>bill = 85.50</span></span>
<span class="line"><span>tip_percent = 18</span></span>
<span class="line"><span>people = 4</span></span>
<span class="line"><span></span></span>
<span class="line"><span>tip = bill * tip_percent / 100 =</span></span>
<span class="line"><span>// Result: 15.39</span></span>
<span class="line"><span></span></span>
<span class="line"><span>total = bill + tip =</span></span>
<span class="line"><span>// Result: 100.89</span></span>
<span class="line"><span></span></span>
<span class="line"><span>per_person = total / people =</span></span>
<span class="line"><span>// Result: 25.22</span></span></code></pre></div><h3 id="area-calculation" tabindex="-1">Area Calculation <a class="header-anchor" href="#area-calculation" aria-label="Permalink to &quot;Area Calculation&quot;">​</a></h3><div class="language- vp-adaptive-theme"><button title="Copy Code" class="copy"></button><span class="lang"></span><pre class="shiki shiki-themes github-light github-dark vp-code" tabindex="0"><code><span class="line"><span>radius = 5</span></span>
<span class="line"><span>area = pi * radius^2 =</span></span>
<span class="line"><span>// Result: 78.54</span></span></code></pre></div>`,39)])])}const b=s(p,[["render",d]]);export{h as __pageData,b as default};
