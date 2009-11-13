*** main template *** (all part outside templates are invisible}
{#template MAIN}
 <div>
  <div>{$T.name.bold()}</div>
  {#include table root=$T.table}
 </div>
{#/template MAIN}

-----------------------------------------

*** main table ***
{#template table}
  <table style="background-color:#CDCDCD;width:100%;max-width:100%;">
    <tr>
    {#foreach $T as r}
      {#include data root=$T.r}
      {#if ($T.r$iteration + 1) % 10 == 0}
        </tr>
	<tr>
      {#/if}
    {#/for}
    </tr>
  </table>
  <br>
{#/template table}

-----------------------------------------

*** for each data ***
{#template data}
  <td style="width:10%;max-width:10%;" 
  {#if $T.status == "Inactive" }
    bgcolor="{'#98AFC7'}"
  {#elseif $T.status == "Bad" }
    bgcolor="{'#FF0000'}"
  {#elseif $T.status == "Warning" }
    bgcolor="{'#FFFF33'}"
  {#elseif $T.status == "Ready" }
    bgcolor="{'#00FF00'}"
  {#elseif $T.status == "Working" } 
    bgcolor="{'#0033FF'}"
  {#else}
    bgcolor="{'#FFFFFF'}"
  {#/if}
  title="
  {$T.name},
  {$T.status},
  {$T.state}
  ">
  <a class="childAttribute" rel="{$T.id}">
  {$T.name}
  </a>
  </td>
{#/template data}
 
