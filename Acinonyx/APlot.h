/*
 *  APlot.h
 *  Acinonyx
 *
 *  Created by Simon Urbanek on 3/4/08.
 *  Copyright 2008 Simon Urbanek. All rights reserved.
 *
 */

// option 1) purely added functionality, the APlotVisual = AVisual + APlot
// option 2) subclass AVisual

#ifndef A_PLOT_H_
#define A_PLOT_H_

#include "AScale.h"
#include "AVisualPrimitive.h"
#include "AContainer.h"
#include "AStack.h"
#include "ADataVector.h"
#include "AQuery.h"

#include "RValueHolder.h" // for variables stored in the plot object

#define SEL_REPLACE 0
#define SEL_OR      1
#define SEL_XOR     2
#define SEL_AND     3
#define SEL_NOT     4

/** AZoomEntry is an interface that represents arbitrary entries on the zoom stack. */
class AZoomEntry : public AObject {
public:
	virtual ADataRange range(vsize_t coord) { return AUndefDataRange; };
};

// FIXME: we are currently only using widgets for delegation, but in principle it could be any superclass
class AWidget;

/** APlot is the base of all visual representing statistical plots. It provides scales, handling of visual primitives, storage for plot primitives (statistical primitives), zoom, query and selection. */
class APlot : public AContainer, public RValueHolder {
protected:
	AScale **_scales;          /**< scales used by this plot */
	int nScales;               /**< number of scales in _scales */
	
	AMutableObjectVector *vps; /**< visual plot primitives (managed externally) */
	AObjectVector *pps;        /**< plot primitives (used by the plot internally - usually statistical visuals) */
	AStack *zoomStack;         /**< stack of zoom definitions */
	AMarker *marker;           /**< subclasses should use this marker if they want primitives to handle selection automatically. It is not used by the APlot class itself (except as pass-through to visual primitives) */
	AQuery *_query;            /**< query object associated with this plot */
	bool inQuery;              /**< flag specifying whether the is currently a query in display */
	
	/** general-purpose status variables - those may not be needed in general, but since most plots use them it makes sense to keep them here */
	AFloat mLeft, mTop, mBottom, mRight, ptSize, ptAlpha;
	const char* _caption;		/**title for the container */
	
	sel_context_t lastContext; /**< last custom selection context */
	sel_context_t currentContext; /**< current context to be used for selection */
	
public:
	APlot(AContainer *parent, ARect frame, int flags) : AContainer(parent, frame, flags), RValueHolder(Rf_allocVector(VECSXP, 0)), nScales(0), pps(NULL), _scales(NULL), vps(new AMutableObjectVector()), zoomStack(new ABlockStack()), marker(0), inSelection(false), inQuery(false), inZoom(false), mLeft(20.0), mTop(10.0), mBottom(20.0), mRight(10.0), ptSize(5.0), ptAlpha(0.6), _caption(NULL), lastContext(SEL_CONTEXT_DEFAULT), currentContext(SEL_CONTEXT_DEFAULT) {
		_query = new AQuery(this);
		_query->setHidden(true);
		add(*_query);
		OCLASS(APlot)
	}

	APlot(AContainer *parent, ARect frame, int flags, AScale *xScale, AScale *yScale) : AContainer(parent, frame, flags), RValueHolder(Rf_allocVector(VECSXP, 0)), nScales(2), pps(NULL), vps(new AMutableObjectVector()), inSelection(false), inZoom(false), inQuery(false), mLeft(20.0), mTop(10.0), mBottom(20.0), mRight(10.0), ptSize(1.0), ptAlpha(1.0), lastContext(SEL_CONTEXT_DEFAULT), currentContext(SEL_CONTEXT_DEFAULT) {
		_scales = (AScale**) AAlloc(sizeof(AScale*)*2);
		AMEM(_scales);
		_scales[0] = xScale;
		_scales[1] = yScale;
		OCLASS(APlot)
	}

	virtual ~APlot() {
		if (_scales) {
			for (int i = 0; i < nScales; i++) _scales[i]->release();
			AFree(_scales);
		}
		if (zoomStack) zoomStack->release();
		if (vps) vps->release();
		if (pps) pps->release();
		DCLASS(APlot)
	}
	
	/** returns the number of scales used in this plot.
	 *  @return number of scales in this plot. */
	vsize_t scales() { return nScales; }
	
	/** returns the scale at the given index. This method is meant mostly for the internal use of the plot. Use designatedScale() to select a scale by its properties.
	 *  @return scale at the given index or NULL in the index is out of bounds */
	AScale *scale(vsize_t index) { return (index < nScales) ? _scales[index] : NULL; }
	
	/** returns a scale by its type. By default index 0 is X scale and index 1 is Y scale, however plots implmenetations can override this
	 *  @param type scale type
	 *  @return scale of the given type or NULL if such scale is not supported by the plot */
	virtual AScale *designatedScale(scale_t type) {
		if (type == XScale && nScales > 0) return _scales[0];
		if (type == YScale && nScales > 1) return _scales[1];
		return NULL;
	}
	
	/** returns the primary marker for the plot (can be NULL). Note that some plots may have multiple markers and others may have none, so the interpretation depends on the semantics in the subclass.
	 *  @return the primary marker or NULL if there is none. */
	AMarker *primaryMarker() { return marker; }

	/** initiates a query at the given point and level. First it resets the query, then queries any internal primitives that contain the point, then queries any visual primitives that contain the point. If there is no text in the query by this point, the plot itself will be queried (and implicitly its children). Finally, if there is still no text, queryOff() is called. Otherwise the query object is made visible.
	 *  @param pt location at which the query should be performed
	 *  @param level level of the query */
	virtual void queryAt(APoint pt, int level) {
		_query->reset();
		_query->move(pt); // after reset we have 0 width, so the location should be precise for all dependents to use
		if (pps) { // query internal plot primitives
			vsize_t n = pps->length();
			for (vsize_t i = 0; i < n; i++) {
				AVisualPrimitive *o = (AVisualPrimitive*) pps->objectAt(i);
				if (o->containsPoint(pt))
					o->query(_query, level);
			}
		}

		if (vps) { // query visual primitives
			vsize_t n = vps->length();
			for (vsize_t i = 0; i < n; i++) {
				AVisualPrimitive *o = (AVisualPrimitive*) vps->objectAt(i);
				if (o->containsPoint(pt))
					o->query(_query, level);
			}
		}

		if (!_query->text()) // if no primitives responded, query the plot itself
			query(_query, level);

		if (_query->text()) { // if the query was successfull, show it
			inQuery = true;
			_query->move(pt); // move again to make sure we fit in the window since our size may have changed
			_query->setHidden(false);
		} else // hide it otherwise
			queryOff();
	}

	/** hides the query */
	virtual void queryOff() {
		bool needRedraw = !_query->isHidden();
		_query->setHidden(true);
		_query->setText(0);
		inQuery = false;
		if (needRedraw) redraw();
	}
	
	/** scale all scales back to "home" scale */
	virtual void home() {
	}
	
	/** update is called when plot properties change (including plot is resized) */
	virtual void update() {
		// update plot primitives
		if (pps) {
			vsize_t i = 0, n = pps->length();
			while (i < n) {
				AVisualPrimitive *o = (AVisualPrimitive*) pps->objectAt(i++);
				if (o) o->update();
			}
		}
		
		// update visual primitives
		if (vps) {
			vsize_t i = 0, n = vps->length();
			while (i < n) {
				AVisualPrimitive *o = (AVisualPrimitive*) vps->objectAt(i++);
				if (o) o->update();
			}
		}
		setRedrawLayer(LAYER_ROOT);
	}

	virtual double doubleProperty(const char *name) {
		AScale *s;
		if (!strcmp(name, "xlim.low") && (s = designatedScale(XScale))) return s->dataRange().begin;
		if (!strcmp(name, "ylim.low") && (s = designatedScale(YScale))) return s->dataRange().begin;
		if (!strcmp(name, "xlim.hi") && (s = designatedScale(XScale))) return s->dataRange().begin + s->dataRange().length;
		if (!strcmp(name, "ylim.hi") && (s = designatedScale(YScale))) return s->dataRange().begin + s->dataRange().length;
		if (!strcmp(name, "num.scales")) return nScales;
		return NA_REAL;
	}
	
	virtual bool setDoubleProperty(const char *name, double value) {
		AScale *s;
		if (!strcmp(name, "xlim.low") && (s = designatedScale(XScale))) { ADataRange r = s->dataRange(); r.begin = value; s->setDataRange(r); update(); redraw(); return true; }
		if (!strcmp(name, "ylim.low") && (s = designatedScale(YScale))) { ADataRange r = s->dataRange(); r.begin = value; s->setDataRange(r); update(); redraw(); return true; }
		if (!strcmp(name, "xlim.hi") && (s = designatedScale(XScale))) { ADataRange r = s->dataRange(); r.length = value - r.begin; s->setDataRange(r); update(); redraw(); return true; }
		if (!strcmp(name, "ylim.hi") && (s = designatedScale(YScale))) { ADataRange r = s->dataRange(); r.length = value - r.begin; s->setDataRange(r); update(); redraw(); return true; }
		return false;
	}
	
	virtual void draw(vsize_t layer) {
		if (layer == LAYER_ROOT || layer == LAYER_HILITE) {
			// draw plot primitives
			if (pps) {
				vsize_t i = 0, n = pps->length();
				while (i < n) {
					AVisualPrimitive *o = (AVisualPrimitive*) pps->objectAt(i++);
					// ALog("%s: draw", o->describe());
					if (!o->hidden()) o->draw(*this, layer);
				}
			}
		}
		
		if (layer == LAYER_OBJECTS) {
			// draw visual primitives
			if (vps) {
				vsize_t i = 0, n = vps->length();
				while (i < n) {
					AVisualPrimitive *o = (AVisualPrimitive*) vps->objectAt(i++);
					ALog("%s: draw", o->describe());
					if (!o->hidden()) o->draw(*this, layer);
				}
			}
		}

		if (layer == LAYER_TRANS) {
			// draw interactive stuff
			if (inSelection || inZoom) {
				ARect r = AMkRect(dragStartPoint.x, dragStartPoint.y, dragEndPoint.x - dragStartPoint.x, dragEndPoint.y - dragStartPoint.y);
				if (r.width < 0) { r.x += r.width; r.width = -r.width; }
				if (r.height < 0) { r.y += r.height; r.height = -r.height; }
				if (inSelection) {
					color(AMkColor(1.0,0.4,0.4,0.5));
				} else if (inZoom)
					color(AMkColor(0.0,0.0,1.0,0.3));				
				rect(r);
				if (inSelection)
					color(AMkColor(1.0,0.0,0.0,1.0));
				else if (inZoom)
					color(AMkColor(0.0,0.0,1.0,1.0));
				rectO(r);			
			}
		}

		// draw all children
		AContainer::draw(layer);
	}

	sel_context_t newContext() {
		return ++lastContext;
	}
	
	/** performs a selection based on a rectangle.
	 *  @param where definition of the graphocal area selected
	 *  @param type type of the selection (one of SEL_REPLACE, SEL_OR, SEL_AND, SEL_XOR and SEL_NOT)
	 *  @param batch whether to perform the selction as a part of a batch (true) or not (false). In batch mode the marker's batch mode won't be changed such that multiple selections can be performed (e.g. selection sequence). The correct usage then is: marker->begin(); performSelection(,,true); ...; marker->end();
	 *  @return whether the selection was performed succesfully (true) or not (false). The latter can essentially happen only if there is no marker or no primitives. */
	virtual bool performSelection(ARect where, int type, bool batch = false) {
		if (!marker) return false;
		if (!vps->length() && (!pps || !pps->length())) return false;
		_prof(profReport("^performSelection %s", describe()))
		if (!batch) marker->begin();
		if (type == SEL_REPLACE)
			marker->deselectAll();
		bool pointSelection = (!where.width && !where.height);
		APoint point = AMkPoint(where.x, where.y);
		ALog("%s performSelection[%g,%g %g,%g] (point selection: %s)", describe(), ARect4(where), pointSelection?"YES":"NO");
		vsize_t i = 0, n;
		if (pps) { // plot primitives
			n = pps->length();
			while (i < n) {
				AVisualPrimitive *vp = (AVisualPrimitive*) pps->objectAt(i++);
				if (vp->context() == currentContext && !vp->hidden() && ((!pointSelection && vp->intersects(where)) || (pointSelection && vp->containsPoint(point))))
					vp->select(marker, type);
			}
		}
		// visual primitives
		i = 0; n = vps->length();
		while (i < n) {
			AVisualPrimitive *vp = (AVisualPrimitive*) vps->objectAt(i++);
			if (vp->context() == currentContext && !vp->hidden() && ((!pointSelection && vp->intersects(where)) || (pointSelection && vp->containsPoint(point))))
				vp->select(marker, type);
		}
		setRedrawLayer(LAYER_HILITE);
		if (!batch) marker->end();
		_prof(profReport("$performSelection %s", describe()))
		return true;
	}
	
	/** the notification is forwarded to all primitives. In addition, N_MarkerChanged causes a redraw from LAYER_HILITE on. */
	virtual void notification(AObject *source, notifid_t nid)
	{
		if (source != this) {
			// notify plot primitives
			if (pps) {
				vsize_t i = 0, n = pps->length();
				while (i < n) {
					AVisualPrimitive *o = (AVisualPrimitive*) pps->objectAt(i++);
					if (o) o->notification(source, nid);
				}
			}
			
			// notify visual primitives
			if (vps) {
				vsize_t i = 0, n = vps->length();
				while (i < n) {
					AVisualPrimitive *o = (AVisualPrimitive*) vps->objectAt(i++);
					if (o) o->notification(source, nid);
				}
			}		

			if (nid == N_TransientMarkerChanged) {
				setRedrawLayer(LAYER_HILITE);
				redraw();
			} else if (nid == N_PermanentMarkerChanged) {
				setRedrawLayer(LAYER_ROOT);
				redraw();
			}

			AContainer::notification(source, nid);
		}
	}

	/** adds a new visual primitive to the plot. AVisualPrimitive::setPlot() is automatically called on the primitive and the redraw layer is set to LAYER_OBJECTS
	 *  @param vp primitive to add (may not be NULL!) */
	void addPrimitive(AVisualPrimitive *vp) {
		vp->setPlot(this);
		vps->addObject(vp);
		setRedrawLayer(LAYER_OBJECTS);
	}
	
	/** remove a visual primitive form the plot
	 *  @param vp primitive to remove (may not be NULL!) */
	void removePrimitive(AVisualPrimitive *vp) {
		if (vp->plot() == this) vp->setPlot(NULL);
		vps->removeObject(vp);
		setRedrawLayer(LAYER_OBJECTS);
	}
	
	/** remove all visual primitives from the plot */
	void removeAllPrimitives() {
		vsize_t n = vps->length();
		for (vsize_t i = 0; i < n; i++) {
			AVisualPrimitive *vp = (AVisualPrimitive*) vps->objectAt(i);
			if (vp) vp->setPlot(NULL);
		}
		vps->removeAll();
		setRedrawLayer(LAYER_OBJECTS);
	}
	
	/** return a vector of all visual primitives used in the plot
	 *  @return vector of all visual primitives. Note that the returned vector shoud be considered immutable! Use addPrimitive()/removePrimitive() to modify the vector instead.
	 *   NOTE: the current implementation does not return a copy, so use with care! */
	AObjectVector *primitives() {
		return vps;
	}
	
	// we should move this to another class - this is here for now ...
	// handling of selection, zoom, etc.
	
	bool inSelection, inZoom;
	APoint dragStartPoint, dragEndPoint;
	int selType;

	virtual bool mouseDown(AEvent e) {
		if (!containsPoint(e.location)) return false;
		if (e.flags & AEF_BUTTON1) {
			if (e.flags & AEF_META)
				inZoom = true;
			else {
				inSelection = true;
				selType = SEL_REPLACE;
				if (e.flags & AEF_SHIFT) {
					selType = SEL_XOR;
					if (e.flags & AEF_ALT)
						selType = SEL_AND;
				}
			}
			dragStartPoint = e.location;
			return true;
		}
		return false;
	}
	
	virtual bool mouseUp(AEvent e) {
		if ((e.flags & AEF_BUTTON1) && (inSelection || inZoom)) {
			dragEndPoint = e.location;
			ARect r = AMkRect(dragStartPoint.x, dragStartPoint.y, dragEndPoint.x - dragStartPoint.x, dragEndPoint.y - dragStartPoint.y);
			if (r.width < 0) { r.x += r.width; r.width = -r.width; }
			if (r.height < 0) { r.y += r.height; r.height = -r.height; }
			if (inSelection) {
				inSelection = false;
				inZoom = false;
				redraw();
				performSelection(r, selType);
			} else if (inZoom) {
				inZoom = false;
				inSelection = false;
				performZoom(r);
				redraw();				
			}
			return true;
		}
		return false;
	}
	
	virtual bool mouseMove(AEvent e) {
		// ALog("%s: mouseMove (flags=%02x, x=%g, y=%g)", describe(), e.flags, e.location.x, e.location.y);
		
		if (AContainer::mouseMove(e)) return true;
		
		if (inQuery) {
			if (e.flags & AEF_CTRL) {
				queryAt(e.location, (e.flags & AEF_SHIFT) ? 1 : 0);
			} else
				queryOff();
		}
		if (inSelection || inZoom) {
			dragEndPoint = e.location;
			redraw();
			return true;
		} else if (e.flags & AEF_CTRL) {
			queryAt(e.location, (e.flags & AEF_SHIFT) ? 1 : 0);
		}
		return false;
	}
	
	virtual bool keyDown(AEvent e) {
		ALog("%s: keyDown(key=%d)", describe(), e.key);
		switch (e.key) {
			case KEY_U: if (marker) marker->undo(); break;
			case KEY_H: {
				vsize_t i = 0, n = marker->length();
				for (; i < n; i++) 
					if (marker->isSelected(i)) break;
				
				if (i < n) {
					marker->begin();
					for (vsize_t i = 0; i < n; i++) 
						if (!marker->isSelected(i))
							marker->hide(i);
					marker->end();
				} else {
					marker->begin();
					for (vsize_t i = 0; i < n; i++)
						marker->show(i);
					marker->end();
				}
				break;
			}
			case KEY_I:
			{
				vsize_t n = marker->length();
				marker->begin();
				for (vsize_t i = 0; i < n; i++)
					if (marker->isSelected(i))
						marker->deselect(i);
					else 
						marker->select(i);
				marker->end();
				break;
			}
			case KEY_A:
			{
				vsize_t n = marker->length();
				marker->begin();
				for (vsize_t i = 0; i < n; i++)
					marker->show(i);
				marker->end();
				break;
			}
			case KEY_X:
				if (lastContext > 0) {
					currentContext++;
					if (currentContext > lastContext)
						currentContext -= (lastContext + 1);
					ALog("%s: set current context to %d (out of %d)", describe(), currentContext, lastContext);
				}
				break;
			case KEY_1:
			{
				vsize_t n = marker->length();
				marker->begin();
				for (vsize_t i = 0; i < n; i++)
					if (marker->isSelected(i))
						marker->setValue(i, 1 + COL_CB1);
				marker->end();
				break;
			}
			case KEY_2:
			{
				vsize_t n = marker->length();
				marker->begin();
				for (vsize_t i = 0; i < n; i++)
					if (marker->isSelected(i))
						marker->setValue(i, 2 + COL_CB1);
				marker->end();
				break;
			}
			case KEY_3:
			{
				vsize_t n = marker->length();
				marker->begin();
				for (vsize_t i = 0; i < n; i++)
					if (marker->isSelected(i))
						marker->setValue(i, 3 + COL_CB1);
				marker->end();
				break;
			}
			case KEY_4:
			{
				vsize_t n = marker->length();
				marker->begin();
				for (vsize_t i = 0; i < n; i++)
					if (marker->isSelected(i))
						marker->setValue(i, 4 + COL_CB1);
				marker->end();
				break;
			}
			case KEY_5:
			{
				vsize_t n = marker->length();
				marker->begin();
				for (vsize_t i = 0; i < n; i++)
					if (marker->isSelected(i))
						marker->setValue(i, 5 + COL_CB1);
				marker->end();
				break;
			}
			default:
				return false;
		}
		return true;
	}
	
	virtual bool performZoom(ARect where) {
		// printf("%s: perform selection: (%g,%g - %g,%g)\n", describe(), where.x, where.y, where.width, where.height);
		return false;
	}
	
	virtual void moveAndResize(ARect frame) {
		if (!ARectsAreEqual(frame, _frame)) {
			AContainer::moveAndResize(frame);
			update();
		}
	}
	
	virtual void delegateAction(AWidget *source, const char *action, AObject *aux) {
		ALog("%s: delegateAction '%s'", describe(), action);
		ALog("   from %s", source ? ((AObject*)source)->describe() : "<unknown!>");
		if (action && !strcmp(action, "undo")) {
			if (marker) marker->undo();
			return;
		}
	}
	
	virtual void setCaption(const char* caption){
		_caption = caption;
		if (_window)
			_window->setTitle(caption);
	}
	
	virtual const char *caption() {
		if (_caption)
			return _caption;
		return "generic plot";
	}
};

class AZoomEntryBiVar : public AZoomEntry {
protected:
	ADataRange r[2];
public:
	AZoomEntryBiVar(ADataRange r1, ADataRange r2) {
		r[0] = r1;
		r[1] = r2;
		OCLASS(AZoomEntryBiVar)
	}

	virtual ADataRange range(vsize_t coord) {
		if (coord == 0) return r[0];
		if (coord == 1) return r[1];
		return AUndefDataRange;
	}
};

#endif
