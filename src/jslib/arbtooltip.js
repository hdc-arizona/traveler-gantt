/* globals d3 */

/** By Alex R Bigelow
 *  https://bl.ocks.org/alex-r-bigelow/b7fce6bf42ff4e910f6695bd48e8a132
 */

/**
  * Shows a tooltip; assumes the existence of a div with `id="tooltip"`, that is
  * positioned absolutely
  * @param  {Object} [parameters={}]
  * Parameter object
  * @param  {String} [parameters.content='']
  * The message that will be displayed. Can be an empty string (hides the
  * tooltip), a non-empty HTML string, or a function (which is called with the
  * d3-selected tooltip div as the first argument)
  * @param  {Object} [parameters.targetBounds=null]
  * Specifies a bounding box that the tooltip should NOT occlude (typically, the
  * results of a getBoundingClientRect() call)
  * @param  {Object} [parameters.anchor=null]
  * Specifies -1 to 1 positioning of the tooltip relative to targetBounds; for
  * example, { x: -1 } right-aligns the tooltip to the left edge of
  * targetBounds, { x: 0 } centers the tooltip horizontally, and { x: 1 }
  * left-aligns the tooltip to the right edge of targetBounds
  * @param {Number} [parameters.hideAfterMs=1000]
  * Hides the tooltip after a certain delay; set this to zero to disable hiding
  */
function showTooltip ({ content = '', targetBounds = null, anchor = null, hideAfterMs = 1000 } = {}) {
  window.clearTimeout(window._tooltipTimeout);
  const showEvent = d3.event;
  d3.select('body').on('click.tooltip', () => {
    if (showEvent !== d3.event) {
      hideTooltip();
    } else {
      d3.event.stopPropagation();
    }
  });

  let tooltip = d3.select('#tooltip')
    .style('left', null)
    .style('top', null)
    .style('width', null)
    .style('display', content ? null : 'none');

  if (content) {
    if (typeof content === 'function') {
      content(tooltip);
    } else {
      tooltip.html(content);
    }
    let tooltipBounds = tooltip.node().getBoundingClientRect();

    let left;
    let top;

    if (targetBounds === null) {
      // todo: position the tooltip WITHIN the window, based on anchor,
      // instead of outside the targetBounds
      throw new Error('tooltips without targetBounds are not yet supported');
    } else {
      anchor = anchor || {};
      if (anchor.x === undefined) {
        if (anchor.y !== undefined) {
          // with y defined, default is to center x
          anchor.x = 0;
        } else {
          if (targetBounds.left > window.innerWidth - targetBounds.right) {
            // there's more space on the left; try to put it there
            anchor.x = -1;
          } else {
            // more space on the right; try to put it there
            anchor.x = 1;
          }
        }
      }
      if (anchor.y === undefined) {
        if (anchor.x !== undefined) {
          // with x defined, default is to center y
          anchor.y = 0;
        } else {
          if (targetBounds.top > window.innerHeight - targetBounds.bottom) {
            // more space above; try to put it there
            anchor.y = -1;
          } else {
            // more space below; try to put it there
            anchor.y = 1;
          }
        }
      }
      left = (targetBounds.left + targetBounds.right) / 2 +
        anchor.x * targetBounds.width / 2 -
        tooltipBounds.width / 2 +
        anchor.x * tooltipBounds.width / 2;
      top = (targetBounds.top + targetBounds.bottom) / 2 +
        anchor.y * targetBounds.height / 2 -
        tooltipBounds.height / 2 +
        anchor.y * tooltipBounds.height / 2;
    }

    // Clamp the tooltip so that it stays on screen
    if (left + tooltipBounds.width > window.innerWidth) {
      left = window.innerWidth - tooltipBounds.width;
    }
    if (left < 0) {
      left = 0;
    }
    if (top + tooltipBounds.height > window.innerHeight) {
      top = window.innerHeight - tooltipBounds.height;
    }
    if (top < 0) {
      top = 0;
    }
    tooltip.style('left', left + 'px')
      .style('top', top + 'px');

    window.clearTimeout(window._tooltipTimeout);
    if (hideAfterMs > 0) {
      window._tooltipTimeout = window.setTimeout(() => {
        hideTooltip();
      }, hideAfterMs);
    }
  }
}
/**
  * Shorthand for hiding the tooltip
  */
function hideTooltip () {
  showTooltip();
}

//export { showTooltip, hideTooltip };
